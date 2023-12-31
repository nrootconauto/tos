#include "tos_aot.hxx"
#include "alloc.hxx"
#include "dbg.hxx"
#include "seth.hxx"

namespace fs = std::filesystem;

using std::ios;

std::unordered_map<std::string, CSymbol> TOSLoader;

namespace {
// This code is mostly copied from TempleOS
void LoadOneImport(u8** src_, u8* module_base) {
  u8* __restrict src = *src_;
  u8*  ptr           = nullptr;
  uptr i             = 0;
  bool first         = true;
  u8   etype;
  // i know this is a GNU extension, problem?
  // this won't actually call memcpy
  // anyway(compiles down to a mov call)
  // so it respects strict aliasing
  // while not compromising on speed
#define READ_NUM(x, T)           \
  ({                             \
    T val_;                      \
    memcpy(&val_, x, sizeof(T)); \
    val_;                        \
  })
  while ((etype = *src++)) {
    ptr = module_base + READ_NUM(src, u32);
    src += sizeof(u32);
    auto st_ptr = (char*)src;
    src += strlen(st_ptr) + 1;
    // First occurance of a string means
    // "repeat this until another name is found"
    if (!*st_ptr)
      goto iet;
    if (!first) {
      *src_ = (u8*)st_ptr - sizeof(u32) - 1;
      return;
    } else {
      first   = false;
      auto it = TOSLoader.find(st_ptr);
      if (it == TOSLoader.end()) {
        fmt::print(stderr, "Unresolved reference {}\n", st_ptr);
        TOSLoader.try_emplace(st_ptr, //
                              /*CSymbol*/ HTT_IMPORT_SYS_SYM, module_base,
                              (u8*)st_ptr - sizeof(u32) - 1);
      } else {
        auto const& [_, sym] = *it;
        if (sym.type != HTT_IMPORT_SYS_SYM)
          i = (uptr)sym.val;
      }
    }
#define OFF(T) ((u8*)i - ptr - sizeof(T))
// same stuff to respect strict aliasing
#define REL(T)                    \
  {                               \
    usize off = OFF(T);           \
    memcpy(ptr, &off, sizeof(T)); \
  }
#define IMM(T) \
  { memcpy(ptr, &i, sizeof(T)); }
  iet:
    switch (etype) {
    case IET_REL_I8:
      REL(i8);
      break;
    case IET_REL_I16:
      REL(i16);
      break;
    case IET_REL_I32:
      REL(i32);
      break;
    case IET_REL_I64:
      REL(i64);
      break;
    case IET_IMM_U8:
      IMM(u8);
      break;
    case IET_IMM_U16:
      IMM(u16);
      break;
    case IET_IMM_U32:
      IMM(u32);
      break;
    case IET_IMM_I64:
      IMM(i64);
      break;
    }
  }
  *src_ = src - 1;
}

void SysSymImportsResolve(char* st_ptr) {
  auto it = TOSLoader.find(st_ptr);
  if (it == TOSLoader.end())
    return;
  auto& [_, sym] = *it;
  if (sym.type != HTT_IMPORT_SYS_SYM)
    return;
  LoadOneImport(&sym.module_header_entry, sym.module_base);
  sym.type = HTT_INVALID;
}

void LoadPass1(u8* src, u8* module_base) {
  u8* ptr;
  u8  etype;
  while ((etype = *src++)) {
    uptr i = READ_NUM(src, u32);
    src += sizeof(u32);
    auto st_ptr = (char*)src;
    src += strlen(st_ptr) + 1;
    switch (etype) {
    case IET_REL32_EXPORT ... IET_IMM64_EXPORT:
      if (etype != IET_IMM32_EXPORT && etype != IET_IMM64_EXPORT)
        i += (uptr)module_base; // i gets reset at the
                                // top of the loop so its fine
      TOSLoader.try_emplace(st_ptr, /*CSymbol*/ HTT_EXPORT_SYS_SYM, (u8*)i);
      SysSymImportsResolve(st_ptr);
      break;
    case IET_REL_I0 ... IET_IMM_I64:
      src = (u8*)st_ptr - sizeof(u32) - 1;
      LoadOneImport(&src, module_base);
      break;
    // 32bit addrs
    case IET_ABS_ADDR:
      for (usize j = 0; j < i /*count*/; j++, src += sizeof(u32)) {
        ptr = module_base + READ_NUM(src, u32);
        // compiles down to `add DWORD PTR[ptr],module_base`
        u32 off;
        memcpy(&off, ptr, sizeof(u32));
        off += (uptr)module_base;
        memcpy(ptr, &off, sizeof(u32));
      }
      break;
      // the other ones wont be used
      // so im not implementing them
    }
  }
}

auto LoadPass2(u8* src, u8* module_base) -> std::vector<void*> {
  std::vector<void*> ret;
  //
  u8 etype;
  while ((etype = *src++)) {
    u32 i = READ_NUM(src, u32);
    src += sizeof(u32);
    src += strlen((char*)src) + 1;
    switch (etype) {
    case IET_MAIN:
      ret.emplace_back(module_base + i);
      break;
    case IET_ABS_ADDR:
      src += sizeof(u32) * i;
      break;
    case IET_CODE_HEAP:
    case IET_ZEROED_CODE_HEAP:
      src += 4 + sizeof(u32) * i;
      break;
    case IET_DATA_HEAP:
    case IET_ZEROED_DATA_HEAP:
      src += 8 + sizeof(u32) * i;
      break;
    }
  }
  return ret;
}

extern "C" struct [[gnu::packed]] CBinFile {
  u16 jmp;
  u8  module_align_bits, reserved /*padding*/;
  union {
    char bin_signature[4];
    u32  sig;
  };
  i64 org, patch_table_offset, file_size;
  u8  data[]; // FAMs are technically illegal in
              // standard c++ but whatever
};

} // namespace

auto LoadHCRT(std::string const& name) -> std::vector<void*> {
  auto f = fopen(name.c_str(), "rb");
  if (!f) {
    fmt::print(stderr, "CANNOT FIND TEMPLEOS BINARY FILE {}\n", name);
    exit(1);
  }
  umax sz;
  if (std::error_code e;
      static_cast<umax>(-1) == (sz = fs::file_size(name, e))) {
    fmt::print(stderr, "CANNOT DETERMINE SIZE OF FILE, ERROR MESSAGE: {}\n",
               e.message());
    fclose(f);
    exit(1);
  }
  u8* bfh_addr;
  fread(bfh_addr = VirtAlloc<u8>(sz), 1, sz, f);
  fclose(f);
  // I think this breaks strict aliasing but
  // I dont think it matters because its packed(?)
  auto bfh = reinterpret_cast<CBinFile*>(bfh_addr);
  if (memcmp(bfh->bin_signature, "TOSB" /*BIN_SIGNATURE_VAL*/, 4)) {
    char wrong_sig[5]{}; // fmt uses strlen for char(&)[N]
                         // when they can clearly just use the arr size :/
    memcpy(wrong_sig, bfh->bin_signature, 4);
    fmt::print(stderr,
               "INVALID TEMPLEOS BINARY FILE, GOT \"{}\" FOR SIGNATURE\n",
               wrong_sig);
    exit(1);
  }
  LoadPass1(bfh_addr + bfh->patch_table_offset, bfh->data);
  return LoadPass2(bfh_addr + bfh->patch_table_offset, bfh->data);
}

// vim: set expandtab ts=2 sw=2 :
