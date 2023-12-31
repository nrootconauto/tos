#include "dbg.hxx"
#include "main.hxx"
#include "seth.hxx"
#include "tos_aot.hxx"
#include "vfs.hxx"

namespace {

struct CCore {
  HANDLE thread, event, mtx;
  // When are we going to wake up at?
  // winmm callback checks for this constantly
  // check out GetTicksHP() for an explanation
  // 0 means it's not sleeping
  u64 awake_at;
  // self-referenced core number
  usize core_num;
  // HolyC function pointers it needs to execute on launch
  std::vector<void*> fps;
};

// look at seth_unix.cxx for an explanation
CCore* cores = nullptr;

thread_local CCore* self;

auto WINAPI ThreadRoutine(LPVOID arg) -> DWORD {
  self = static_cast<CCore*>(arg);
  VFsThrdInit();
  SetupDebugger();
  for (auto fp : self->fps) {
    FFI_CALL_TOS_0_ZERO_BP(fp);
  }
  return 0;
}

usize proc_cnt;

thread_local void* Fs = nullptr;
thread_local void* Gs = nullptr;

} // namespace

auto GetFs() -> void* {
  return Fs;
}

void SetFs(void* f) {
  Fs = f;
}

auto GetGs() -> void* {
  return Gs;
}

void SetGs(void* g) {
  Gs = g;
}

auto CoreNum() -> usize {
  return self->core_num;
}

// this may look like bad code but HolyC cannot switch
// contexts unless you call Yield() in a loop(eg, `while(TRUE);`)
// so we have to set RIP manually(this routine is called
// when CTRL+ALT+C/X is pressed inside TempleOS while the core is
// stuck in an infinite loop witout yielding)
void InterruptCore(usize core) {
  auto&   c = cores[core];
  CONTEXT ctx{};
  ctx.ContextFlags = CONTEXT_FULL;
  SuspendThread(c.thread);
  GetThreadContext(c.thread, &ctx);
  // push rip
  memcpy((void*)(ctx.Rsp -= 8), &ctx.Rip, 8); // save current program counter
  static void* fp = nullptr;
  if (!fp)
    fp = TOSLoader["__InterruptCoreRoutine"].val;
  // movabs rip, <fp>
  ctx.Rip = reinterpret_cast<uptr>(fp); // set new program counter
  SetThreadContext(c.thread, &ctx);
  ResumeThread(c.thread);
}

void CreateCore(std::vector<void*>&& fps) {
  static usize core_num = 0;
  if (!cores) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    auto mp_proc_cnt = si.dwNumberOfProcessors;
    //
    cores = new (std::nothrow) CCore[proc_cnt = //
                                     std::min<usize>(mp_proc_cnt, 128)];
  }
  auto& c    = cores[core_num];
  c.fps      = std::move(fps);
  c.core_num = core_num;
  c.thread   = CreateThread(nullptr, 0, ThreadRoutine, &c, 0, nullptr);
  c.mtx      = CreateMutex(nullptr, FALSE, nullptr);
  c.event    = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  SetThreadPriority(c.thread, THREAD_PRIORITY_HIGHEST);
  ++core_num;
  // I wanted to set thread names but literally wtf,
  // also gcc doesnt support it so whatever
  // https://archive.li/MIMDo
}

#define LOCK_STORE(dst, val) __atomic_store_n(&dst, val, __ATOMIC_SEQ_CST)

void AwakeCore(usize core) {
  auto& c = cores[core];
  // the timeSetEvent callback will constantly check
  // if the total ticks have reached awake_at(when we want to wake up)
  // so we simply call WaitForSingleObject()
  WaitForSingleObject(c.mtx, INFINITE);
  // awake_at will be automatically set to 0
  SetEvent(c.event);
  ReleaseMutex(c.mtx);
}

namespace {

UINT tick_inc;
u64  ticks = 0;

void UpdateTicksCB(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR) {
  ticks += tick_inc;
  for (usize i = 0; i < proc_cnt; ++i) {
    auto& c = cores[i];
    WaitForSingleObject(c.mtx, INFINITE);
    // check if ticks reached awake_at
    if (ticks >= c.awake_at && c.awake_at > 0) {
      SetEvent(c.event);
      LOCK_STORE(c.awake_at, 0);
    }
    ReleaseMutex(c.mtx);
  }
}

// To just get ticks we can use QueryPerformanceFrequency
// and QueryPerformanceCounter but we want to set an winmm
// event that updates the tick count while also helping cores wake up
//
// i killed two birds with one stoner
auto GetTicksHP() -> u64 {
  static bool first_run = true;
  if (first_run) {
    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof tc);
    tick_inc = tc.wPeriodMin;
    timeSetEvent(tick_inc, tick_inc, UpdateTicksCB, 0, TIME_PERIODIC);
    first_run = false;
  }
  return ticks;
}

} // namespace

void SleepHP(u64 us) {
  auto&      c = cores[CoreNum()];
  auto const t = GetTicksHP(); // milliseconds
  WaitForSingleObject(c.mtx, INFINITE);
  // windows doesnt have accurate microsecond sleep :(
  c.awake_at = t + us / 1000;
  ReleaseMutex(c.mtx);
  WaitForSingleObject(c.event, INFINITE);
}

// vim: set expandtab ts=2 sw=2 :
