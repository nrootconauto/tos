# ffi
cpp2holyc.cxx
```C++
// inside the anonymous namespace below "// ffi functions go below here"
// please use trailing return types
auto STK_FunctionName(u64* stk) -> u64 {
  // ...
}
// in BootstrapLoader()
// RegisterFunctionPtrs({ ...
       S(FunctionName, <function arg cnt in HolyC>),
// });
```
`STK_FunctionName` ***MUST*** return void OR a value that is 8 bytes big

T/KERNELA.HH
```C
...after #ifdef IMPORT_BUILTINS
import U64 FunctionName(<args>);
...#else then lots of extern
extern <same function prototype>;
```
build hcrt and loader again, copy HCRT.BIN to HCRT\_BOOTSTRAP.BIN and commit
# extending the kernel
T/KERNELA.HH
```C
//same as ffi without the "import" line
```
T/HCRT\_TOS.HC
```C
#include "<desired holyc file>"
```
rebuild hcrt, copy HCRT.BIN to HCRT\_BOOTSTRAP.BIN and commit
# header generation
T/FULL\_PACKAGE.HC
```C
#define GEN_HEADERS 1
```
make -> run tine -> T/unfound.DD
```
<functions>
```
copy desired fn prototypes to T/KERNELA.HH
