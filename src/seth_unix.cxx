#include "dbg.hxx"
#include "main.hxx"
#include "seth.hxx"
#include "tos_aot.hxx"
#include "vfs.hxx"

namespace {

// note: these cores will never be destructed because this is
// basically an emulation of a real CPU core, CoreAPSethTask never terminates,
// instead let the host OS clean it up and do whatever with it
struct CCore {
  pthread_t thread;
  /*
   * clang-format off
   * man 2 futex
   * > The uaddr argument points to the futex word.  On all platforms,
   * > futexes are four-byte integers that must be aligned on a four-
   * > byte boundary.
   * freebsd doesnt seem to mind about alignment so im just going to use u32 too
   * (though i have to specify UMTX_OP_WAIT_UINT instead of UMTX_OP_WAIT)
   * clang-format on
   */
  // Is this thread sleeping?
  //
  // not using std::atomic<T> here
  // also std::atomic_ref<T> isn't implemented in libc++ yet
  // so i will be using atomic builtins
  alignas(4) u32 is_sleeping;
  // self-referenced core number
  usize core_num;
  // HolyC function pointers it needs to execute on launch
  // (especially important for Core 0)
  std::vector<void*> fps;
};

CCore* cores = nullptr;
// Thread local self-referenced CCore structure
thread_local CCore* self;

auto ThreadRoutine(void* arg) -> void* {
  VFsThrdInit();
  SetupDebugger();
  static void* sig_fp = nullptr;
  if (!sig_fp)
    sig_fp = TOSLoader["__InterruptCoreRoutine"].val;
  signal(SIGUSR1, reinterpret_cast<SignalCallback*>(sig_fp));
  self = static_cast<CCore*>(arg);
  // CoreAPSethTask(...) (T/FULL_PACKAGE.HC) <- (non-Core0)
  // IET_MAIN boot functions + kernel entry point <- Core0
  //
  // ZERO_BP so the return addr&rbp is 0 and
  // stack traces don't climb up the C++ stack
  for (auto fp : self->fps) {
    FFI_CALL_TOS_0_ZERO_BP(fp);
  }
  // Note: CoreAPSethTask() will NEVER return
  return nullptr;
}

/*
 * (DolDoc code)
 * $ID,-2$$TR-C,"How do you use the FS and GS segment registers."$
 * $ID,2$$FG,2$MOV RAX,FS:[RAX]$FG$ : FS can be set with a $FG,2$WRMSR$FG$,
 * but displacement is RIP relative, so it's tricky to use.  FS is used for
 * the current $LK,"CTask",A="MN:CTask"$, GS for $LK,"CCPU",A="MN:CCPU"$.
 *
 * Note on Fs and Gs: They might seem like very weird names for ThisTask and
 * ThisCPU repectively but it's because they are stored in the F Segment and G
 * Segment registers in native TempleOS. (https://archive.md/pf2td)
 */
// I tried putting this in CCore but it became fucky wucky so yeah, it's here
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

void InterruptCore(usize core) {
  auto& c = cores[core];
  // block signals temporarily
  // will be unblocked later by __InterruptCoreRoutine
  sigset_t all;
  sigfillset(&all);
  sigprocmask(SIG_BLOCK, &all, nullptr);
  // this will execute the signal handler for SIGUSR1 in the core because i cant
  // remotely suspend threads like Win32 SuspendThread in unix
  pthread_kill(c.thread, SIGUSR1);
}

void CreateCore(std::vector<void*>&& fps) {
  static usize core_num = 0;
  // TempleOS has a hardcoded maximum core count
  // of 128(MP_PROCESSORS_NUM)
  //
  // boot
  if (!cores)
    cores = new (std::nothrow) CCore[std::min<usize>( //
        sysconf(_SC_NPROCESSORS_ONLN), 128)];
  auto& c = cores[core_num];
  // CoreAPSethTask(...) passed from SpawnCore or
  // IET_MAIN function pointers+kernel entry point from LoadHCRT
  c.fps      = std::move(fps);
  c.core_num = core_num;
  pthread_create(&c.thread, nullptr, ThreadRoutine, &c);
  char buf[16]{};
  fmt::format_to_n(buf, 15, "Seth(Core{})", c.core_num);
  pthread_setname_np(c.thread, buf);
  ++core_num;
  // pthread_setname_np only works on glibc and FreeBSD
  // on OpenBSD it's pthread_set_name_np. damn, Theo.
}

#define LOCK_STORE(dst, val) __atomic_store_n(&dst, val, __ATOMIC_SEQ_CST)

#ifdef __linux__
  #define AWAKE(core_stat) \
    syscall(SYS_futex, &core_stat, FUTEX_WAKE, UINT32_C(1), nullptr, nullptr, 0)
#elif defined(__FreeBSD__)
  #define AWAKE(core_stat) \
    _umtx_op(&core_stat, UMTX_OP_WAKE, UINT32_C(1), nullptr, nullptr)
#endif

void AwakeCore(usize core) {
  auto& c = cores[core];
  if (c.is_sleeping)
    AWAKE(c.is_sleeping);
  LOCK_STORE(c.is_sleeping, UINT32_C(0));
}

#ifdef __linux__
  #define SLEEP_FOR(core_stat, val, timeout) \
    syscall(SYS_futex, &core_stat, FUTEX_WAIT, val, &timeout, nullptr, 0)
#elif defined(__FreeBSD__)
  #define SLEEP_FOR(core_stat, val, timeout)     \
    _umtx_op(&core_stat, UMTX_OP_WAIT_UINT, val, \
             (void*)sizeof(struct timespec), &timeout)
#endif

void SleepHP(u64 us) {
  auto&           c = cores[CoreNum()];
  struct timespec ts {};
  ts.tv_nsec = (us % 1000000) * 1000;
  ts.tv_sec  = us / 1000000;
  LOCK_STORE(c.is_sleeping, UINT32_C(1));
  SLEEP_FOR(c.is_sleeping, UINT32_C(1), ts);
  LOCK_STORE(c.is_sleeping, UINT32_C(0));
}

// vim: set expandtab ts=2 sw=2 :
