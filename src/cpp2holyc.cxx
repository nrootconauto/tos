#include "alloc.hxx"
#include "holyc_routines.hxx"
#include "main.hxx"
#include "sdl_window.hxx"
#include "seth.hxx"
#include "sound.h"
#include "tos_aot.hxx"
#include "tosprint.hxx"
#include "vfs.hxx"

namespace {

namespace chrono = std::chrono;

using chrono::system_clock;

namespace {

struct HolyFunc {
  std::string_view m_name;
  uptr             m_fp;
  u16              m_arity; // arity must be <= 0xffFF/sizeof U64
};

template <usize S> struct ByteLiteral {
  usize       m_sz;
  char const* m_lit;
  // char is fine here because we aren't doing any arithmetic on them
  using StrLit = char[S];
  consteval ByteLiteral(StrLit const& s) //
      : m_sz{S - 1}, m_lit{s}            //
  {}
};

void RegisterFunctionPtrs(std::initializer_list<HolyFunc> ffi_list) {
  // clang-format off
  // I used https://defuse.ca/online-x86-assembler.htm
  // boring register pushing and stack alignment bullshit
  // read the SysV/Win64 ABIs and Doc/GuideLines.DD if interested
  // below is a criminal-grade string literal abuse scene to avoid extra
  // allocations like a vector of std::string's(my previous approach)
  /* the reason i pack all the machine instructions into
   * one string literal is because i want simd instructions to move it
   * quickly and modify only a small portion of it
   */
  ByteLiteral constexpr inst =
  /*
   * 0x0:  55                      push   rbp
   * 0x1:  48 89 e5                mov    rbp,rsp
   * 0x4:  48 83 e4 f0             and    rsp,0xfffffffffffffff0
   *                          // ^ chops stack off to align to 16
   *                          // (ABI requirement in both Win32/SysV)
   * 0x8:  56                      push   rsi
   * 0x9:  57                      push   rdi
   * 0xa:  41 52                   push   r10
   * 0xc:  41 53                   push   r11
   * 0xe:  41 54                   push   r12
   * 0x10: 41 55                   push   r13
   * 0x12: 41 56                   push   r14
   * 0x14: 41 57                   push   r15
   * len = 0x16
   */
        "\x55"
        "\x48\x89\xE5"
        "\x48\x83\xE4\xF0"
        "\x56"
        "\x57"
        "\x41\x52"
        "\x41\x53"
        "\x41\x54"
        "\x41\x55"
        "\x41\x56"
        "\x41\x57"
#ifdef _WIN32
  // https://archive.md/4HDA0#selection-2085.880-2085.1196
  // rcx is the first arg i have to provide in win64 abi
  // win64 abi requires me to push 4 register parameters
  // even though im not using all of them
  /*
   * 0x0:  48 8d 4d 10             lea    rcx,[rbp+0x10]
   * 0x4:  41 51                   push   r9
   * 0x6:  41 50                   push   r8
   * 0x8:  52                      push   rdx
   * 0x9:  51                      push   rcx
   * len = 0xa
   */
        "\x48\x8D\x4D\x10"
        "\x41\x51"
        "\x41\x50"
        "\x52"
        "\x51"
#else // sysv
  // rdi is the first arg i have to provide in sysv
  /*
   * 0x0:  48 8d 7d 10             lea    rdi,[rbp+0x10]
   * len = 0x4
   */
        "\x48\x8D\x7D\x10"
#endif
  /*
   * 0x0:  48 b8 11 22 33 44 55 66 77 88    movabs rax,0x8877665544332211(fp)
   * 0xa:  ff d0                            call   rax
   * len = 0xc
   */
        "\x48\xB8" "\x11\x22\x33\x44\x55\x66\x77\x88" // 0x8877... is a placeholder
        "\xFF\xD0"
#ifdef _WIN32
  // can just add to rsp since
  // those 4 registers are volatile
  /* 0x0:  48 83 c4 20             add    rsp,0x20
   * len = 0x4
   */
        "\x48\x83\xC4\x20"
#endif
  // pops stack. boring stuff
  /*
   * 0x0:  41 5f                   pop    r15
   * 0x2:  41 5e                   pop    r14
   * 0x4:  41 5d                   pop    r13
   * 0x6:  41 5c                   pop    r12
   * 0x8:  41 5b                   pop    r11
   * 0xa:  41 5a                   pop    r10
   * 0xc:  5f                      pop    rdi
   * 0xd:  5e                      pop    rsi
   * 0xe:  c9                      leave
   * 0xf:  c2 11 22                ret    0x2211(arity*8(==sizeof u64))
   * len = 0x12
   */
        "\x41\x5F"
        "\x41\x5E"
        "\x41\x5D"
        "\x41\x5C"
        "\x41\x5B"
        "\x41\x5A"
        "\x5F"
        "\x5E"
        "\xC9"
        "\xC2" "\x11\x22"; // 0x2211 is a placeholder
  // clang-format on
  auto constexpr fp_off =
#ifndef _WIN32
      0x16 + 0x4 + 0x2;
#else
      0x16 + 0xa + 0x2;
#endif
  auto constexpr arity_off = inst.m_sz - 2;
  //
  auto blob = VirtAlloc<u8>(inst.m_sz * ffi_list.size());
  for (usize i = 0; i < ffi_list.size(); ++i) {
    u8* cur_pos = blob + i * inst.m_sz;
    memcpy(cur_pos, inst.m_lit, inst.m_sz);
    auto const& hf = ffi_list.begin()[i];
    // for the 0x8877... placeholder
    memcpy(cur_pos + fp_off, &hf.m_fp, 8);
    // for the 0x2211 placeholder(ret imm16)
    // all args are 64bit in HolyC, so *8
    u16 arity = hf.m_arity * 8;
    memcpy(cur_pos + arity_off, &arity, 2);
    TOSLoader.try_emplace(std::string{hf.m_name}, /*CSymbol*/ HTT_FUN, cur_pos);
  }
}
// clang-format off
// ret <arity*8>; (8 == sizeof(u64))
// HolyC ABI is __stdcall, the callee cleans up its own stack
// unless its variadic so we pop the stack with ret
//
// A bit about HolyC ABI: all args are 8 bytes(64 bits)
// let there be function Foo(I64 i, ...);
// call Foo() like Foo(2, 4, 5, 6);
// stack view:
//   argv[2]  6 // RBP + 48
//   argv[1]  5 // RBP + 40
//   argv[0]  4 // RBP + 32 <-points- argv (internal var in function)
//   argc     3 // RBP + 24 <-value-- argc (internal var in function, num of varargs) 
//   i        2 // RBP + 16 this is where the argument stack starts
//   0x???????? // RBP + 8  return address
//   0x???????? // RBP + 0  previous RBP of caller function
// clang-format on

auto constexpr StrHash(std::string_view sv) -> u64 { // fnv64-1a
  u64 h = 0xCBF29CE484222325;
  for (auto c : sv) {
    h *= 0x100000001B3;
    h ^= static_cast<u8>(c);
  }
  return h;
}
} // namespace

// ffi functions go below here
auto STK_mp_cnt(void*) -> usize {
  static usize proc_cnt  = 0;
  static bool  first_run = true;
  if (first_run) {
#ifndef _WIN32
    proc_cnt = sysconf(_SC_NPROCESSORS_ONLN);
#else
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    proc_cnt = si.dwNumberOfProcessors;
#endif
    first_run = false;
  }
  return std::min<usize>(proc_cnt, 128);
}

void STK_DyadInit(void*) {
  static bool init = false;
  if (init)
    return;
  init = true;
  dyad_init();
  dyad_setUpdateTimeout(0.);
}

auto STK_DyadListen(iptr* stk) -> i64 {
  return dyad_listen((dyad_Stream*)stk[0], (int)stk[1]);
}

auto STK_DyadConnect(iptr* stk) -> i64 {
  return dyad_connect((dyad_Stream*)stk[0], (char*)stk[1], (int)stk[2]);
}

void STK_DyadWrite(iptr* stk) {
  dyad_write((dyad_Stream*)stk[0], (void*)stk[1], (int)stk[2]);
}

void STK_DyadEnd(dyad_Stream** stk) {
  dyad_end(stk[0]);
}

void STK_DyadClose(dyad_Stream** stk) {
  dyad_close(stk[0]);
}

auto STK_DyadGetAddress(dyad_Stream** stk) -> char* {
  return HolyStrDup(dyad_getAddress(stk[0]));
}

auto STK__DyadGetCallbackMode(char** stk) -> i64 {
  // i thought of using streamprint but then
  // the variadic calling thing gets too complicated
  switch (StrHash(stk[0])) {
    // by default it's std::underlying_type_t<decltype(DYAD_EVENT_CLOSE)>
    // so we static_cast just in case
#define C(x)        \
  case StrHash(#x): \
    return static_cast<i64>(x)
    C(DYAD_EVENT_LINE);
    C(DYAD_EVENT_DATA);
    C(DYAD_EVENT_CLOSE);
    C(DYAD_EVENT_CONNECT);
    C(DYAD_EVENT_DESTROY);
    C(DYAD_EVENT_ERROR);
    C(DYAD_EVENT_READY);
    C(DYAD_EVENT_TICK);
    C(DYAD_EVENT_TIMEOUT);
    C(DYAD_EVENT_ACCEPT);
#undef C
  default:
    HolyThrow("InvMode"); // invalid mode
  }
}

void STK_DyadSetReadCallback(void** stk) {
  dyad_addListener((dyad_Stream*)stk[0], (iptr)stk[1],
                   [](dyad_Event* e) {
                     FFI_CALL_TOS_4(e->udata, (uptr)e->stream, (uptr)e->data,
                                    e->size, (uptr)e->udata2);
                   },
                   stk[2], stk[3]);
}

void STK_DyadSetCloseCallback(void** stk) {
  dyad_addListener((dyad_Stream*)stk[0], (iptr)stk[1],
                   [](dyad_Event* e) {
                     FFI_CALL_TOS_2(e->udata, (uptr)e->stream, (uptr)e->udata2);
                   },
                   stk[2], stk[3]);
}

void STK_DyadSetListenCallback(void** stk) {
  dyad_addListener((dyad_Stream*)stk[0], (iptr)stk[1],
                   [](dyad_Event* e) {
                     FFI_CALL_TOS_2(e->udata, (uptr)e->remote, (uptr)e->udata2);
                   },
                   stk[2], stk[3]);
}

// read callbacks -> DYAD_EVENT_{LINE,DATA}
// close callbacks -> DYAD_EVENT_{DESTROY,ERROR,CLOSE}
// listen callbacks -> DYAD_EVENT_{TIMEOUT,CONNECT,READY,TICK,ACCEPT}

void STK_DyadSetTimeout(uptr* stk) {
  dyad_setTimeout((dyad_Stream*)stk[0], std::bit_cast<f64>(stk[1]));
}

void STK_DyadSetNoDelay(iptr* stk) {
  dyad_setNoDelay((dyad_Stream*)stk[0], (int)stk[1]);
}

void STK_UnblockSignals(void*) {
#ifndef _WIN32
  sigset_t all;
  sigfillset(&all);
  sigprocmask(SIG_UNBLOCK, &all, nullptr);
#endif
}

void STK__GrPaletteColorSet(u64* stk) {
  GrPaletteColorSet(stk[0], bgr_48{stk[1]});
}

auto STK___IsValidPtr(uptr* stk) -> u64 {
#ifdef _WIN32
  // return !IsBadReadPtr((void*)stk[0], 8);
  // wtf IsBadReadPtr gives me a segfault so i use a polyfill
  MEMORY_BASIC_INFORMATION mbi{};
  if (VirtualQuery((void*)stk[0], &mbi, sizeof mbi)) {
    // https://archive.md/ehBq4
    DWORD mask = PAGE_READONLY          //
               | PAGE_READWRITE         //
               | PAGE_WRITECOPY         //
               | PAGE_EXECUTE           //
               | PAGE_EXECUTE_READ      //
               | PAGE_EXECUTE_READWRITE //
               | PAGE_EXECUTE_WRITECOPY;
    return !!(mbi.Protect & mask);
  }
  return false;
#else
  static bool  first_run = true;
  static usize page_size;
  if (first_run) {
    page_size = sysconf(_SC_PAGESIZE);
    first_run = false;
  }
  /* round down to page boundary (equiv to stk[0] / page_size * page_size)
   *   0b100101010 (x)
   * & 0b111110000 <- ~(0b10000 - 1)
   * --------------
   *   0b100100000
   */
  // https://archive.md/Aj0S4
  uintptr_t ptr = stk[0] & ~(page_size - 1);
  return -1 != msync((void*)ptr, page_size, MS_ASYNC);

#endif
}

void STK_InterruptCore(usize* stk) {
  InterruptCore(stk[0]);
}

void STK___BootstrapForeachSymbol(void** stk) {
  for (auto& [name, sym] : TOSLoader)
    FFI_CALL_TOS_3(stk[0], (uptr)name.c_str(), (uptr)sym.val,
                   sym.type == HTT_EXPORT_SYS_SYM ? HTT_FUN : sym.type);
}

void STK_TOSPrint(iptr* stk) {
  TOSPrint((char*)stk[0], stk[1], stk + 2);
}

void STK_DrawWindowUpdate(u8** stk) {
  DrawWindowUpdate(stk[0]);
}

// microseconds
auto STK___GetTicksHP(void*) -> u64 {
#ifndef _WIN32
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<u64>(ts.tv_nsec) / UINT64_C(1000) //
       + static_cast<u64>(ts.tv_sec) * UINT64_C(1000000);
#else
  static u64 freq = 0;
  if (!freq) {
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    freq /= UINT64_C(1000000);
  }
  u64 cur;
  QueryPerformanceCounter((LARGE_INTEGER*)&cur);
  return cur / freq;
#endif
}

// milliseconds
auto STK___GetTicks(void*) -> u64 {
#ifndef _WIN32
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<u64>(ts.tv_nsec) / UINT64_C(1000000) //
       + static_cast<u64>(ts.tv_sec) * UINT64_C(1000);
#else
  return GetTickCount();
#endif
}

void STK_SetKBCallback(void** stk) {
  SetKBCallback(stk[0]);
}

void STK_SetMSCallback(void** stk) {
  SetMSCallback(stk[0]);
}

void STK___AwakeCore(usize* stk) {
  AwakeCore(stk[0]);
}

void STK___SleepHP(u64* stk) {
  SleepHP(stk[0]);
}

void STK___Sleep(u64* stk) {
  SleepHP(stk[0] * 1000);
}

void STK_SetFs(void** stk) {
  SetFs(stk[0]);
}

void STK_SetGs(void** stk) {
  SetGs(stk[0]);
}

void STK_SndFreq(u64* stk) {
  SndFreq(stk[0]);
}

void STK_SetClipboardText(char** stk) {
  SetClipboard(stk[0]);
}

auto STK___GetStr(char** stk) -> char* {
  char *s = linenoise(stk[0]), *r;
  if (!s)
    return nullptr;
  linenoiseHistoryAdd(s);
  r = HolyStrDup(s);
  free(s);
  return r;
}

auto STK_FUnixTime(char** stk) -> u64 {
  return VFsFUnixTime(stk[0]);
}

auto STK_UnixNow(void*) -> u64 {
  return system_clock::to_time_t(system_clock::now());
}

void STK___SpawnCore(uptr* stk) {
  CreateCore(std::vector<void*>{
      (void*)stk[0],
  });
}

auto STK_NewVirtualChunk(usize* stk) -> void* {
  return NewVirtualChunk(stk[0], stk[1]);
}

void STK_FreeVirtualChunk(uptr* stk) {
  FreeVirtualChunk((void*)stk[0], stk[1]);
}

void STK_VFsSetPwd(char** stk) {
  VFsSetPwd(stk[0]);
}

auto STK_VFsFExists(char** stk) -> u64 {
  return VFsFExists(stk[0]);
}

auto STK_VFsIsDir(char** stk) -> u64 {
  return VFsIsDir(stk[0]);
}

auto STK_VFsFSize(char** stk) -> i64 {
  return VFsFSize(stk[0]);
}

void STK_VFsFTrunc(uptr* stk) {
  VFsFTrunc((char*)stk[0], stk[1]);
}

auto STK_VFsFRead(char** stk) -> u8* {
  return VFsFRead(stk[0], (u64*)stk[1]);
}

auto STK_VFsFWrite(char** stk) -> u64 {
  return VFsFWrite(stk[0], stk[1], (uptr)stk[2]);
}

auto STK_VFsDirMk(char** stk) -> u64 {
  return VFsDirMk(stk[0]);
}

auto STK_VFsDir(void*) -> char** {
  return VFsDir();
}

auto STK_VFsDel(char** stk) -> u64 {
  return VFsDel(stk[0]);
}

auto STK_VFsFOpenW(char** stk) -> FILE* {
  return VFsFOpen(stk[0], "w+b");
}

auto STK_VFsFOpenR(char** stk) -> FILE* {
  return VFsFOpen(stk[0], "rb");
}

void STK_VFsFClose(FILE** stk) {
  fclose(stk[0]);
}

auto STK_VFsFBlkRead(uptr* stk) -> u64 {
  fflush((FILE*)stk[3]);
  auto bytes = stk[1] * stk[2];
  return bytes == fread((void*)stk[0], 1, bytes, (FILE*)stk[3]);
}

auto STK_VFsFBlkWrite(uptr* stk) -> u64 {
  auto bytes = stk[1] * stk[2];
  bool r     = bytes == fwrite((void*)stk[0], 1, bytes, (FILE*)stk[3]);
  fflush((FILE*)stk[3]);
  return r;
}

auto STK_VFsFSeek(uptr* stk) -> u64 {
  return 0 != fseek((FILE*)stk[1], stk[0], SEEK_SET);
}

void STK_VFsSetDrv(u8* stk) {
  VFsSetDrv(stk[0]);
}

void STK_SetVolume(u64* stk) {
  union {
    u64 i;
    f64 flt;
  } un = {stk[0]};
  SetVolume(un.flt);
}

void STK_ExitTINE(int* stk) {
  _Exit(stk[0]);
}

} // namespace

void BootstrapLoader() {
#define R(holy, secular, arity) {holy, (uptr)secular, arity}
#define S(name, arity) \
  { #name, (uptr)STK_##name, arity }
  RegisterFunctionPtrs({
      R("__CmdLineBootText", CmdLineBootText, 0),
      R("__CoreNum", CoreNum, 0),
      R("GetFs", GetFs, 0),
      R("GetGs", GetGs, 0),
      S(mp_cnt, 0),
      R("__IsCmdLine", IsCmdLine, 0),
      S(__IsValidPtr, 1),
      S(__SpawnCore, 1),
      S(UnixNow, 0),
      S(InterruptCore, 1),
      S(NewVirtualChunk, 2),
      S(FreeVirtualChunk, 2),
      S(ExitTINE, 1),
      S(__GetStr, 1),
      S(FUnixTime, 1),
      R("GetClipboardText", ClipboardText, 0),
      S(SetClipboardText, 1),
      S(SndFreq, 1),
      S(__Sleep, 1),
      S(__SleepHP, 1),
      S(__AwakeCore, 1),
      S(SetFs, 1),
      S(SetGs, 1),
      S(SetKBCallback, 1),
      S(SetMSCallback, 1),
      S(__GetTicks, 0),
      S(__BootstrapForeachSymbol, 1),
      S(DrawWindowUpdate, 1),
      R("DrawWindowNew", DrawWindowNew, 0),
      R("PCSpkInit", PCSpkInit, 0),
      S(UnblockSignals, 0),
      /*
       * in TempleOS variadics, functions follow __cdecl, whereas normal ones
       * follow __stdcall which is why the arity argument is needed(ret imm16)
       * thus we don't have to clean up the stack in variadics
       */
      S(TOSPrint, 0),
      //
      S(DyadInit, 0),
      R("DyadUpdate", dyad_update, 0),
      R("DyadShutdown", dyad_shutdown, 0),
      R("DyadNewStream", dyad_newStream, 0),
      S(DyadListen, 2),
      S(DyadConnect, 3),
      S(DyadWrite, 3),
      S(DyadEnd, 1),
      S(DyadClose, 1),
      S(DyadGetAddress, 1),
      S(_DyadGetCallbackMode, 1),
      S(DyadSetReadCallback, 4),
      S(DyadSetCloseCallback, 4),
      S(DyadSetListenCallback, 4),
      S(DyadSetTimeout, 2),
      S(DyadSetNoDelay, 2),
      S(VFsFTrunc, 2),
      S(VFsSetPwd, 1),
      S(VFsFExists, 1),
      S(VFsIsDir, 1),
      S(VFsFSize, 1),
      S(VFsFRead, 2),
      S(VFsFWrite, 3),
      S(VFsDel, 1),
      S(VFsDir, 0),
      S(VFsDirMk, 1),
      S(VFsFBlkRead, 4),
      S(VFsFBlkWrite, 4),
      S(VFsFOpenW, 1),
      S(VFsFOpenR, 1),
      S(VFsFClose, 1),
      S(VFsFSeek, 2),
      S(VFsSetDrv, 1),
      R("VFsGetDrv", VFsGetDrv, 0),
      R("GetVolume", GetVolume, 0),
      S(SetVolume, 1),
      S(__GetTicksHP, 0),
      S(_GrPaletteColorSet, 2),
  });
}

// vim: set expandtab ts=2 sw=2 :
