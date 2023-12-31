cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

option(BUILD_HCRT "Build HolyC kernel when loader is built" ON)

# do not fuck crazy
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "You cannot use CMake to build from the root of its source tree! Remove the CMakeCache.txt file and CMakeFiles directory from this directory, then create a separate directory (either below this directory or elsewhere), and then re-run CMake from there.")
endif ()

# lmao lol
set(PROJECT_MAJOR_VERSION 4)
set(PROJECT_MINOR_VERSION 2)
set(PROJECT_PATCH_LEVEL   0)
set(VERSION_SIMPLE "${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL}")

project(tine
  VERSION ${VERSION_SIMPLE}
  DESCRIPTION "TINE Is Not an Emulator"
  LANGUAGES CXX C ASM
)

set(default_build_type "Release")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}"
    CACHE STRING "Choose the type of build." FORCE
  )
  set_property(CACHE CMAKE_BUILD_TYPE
    PROPERTY
      STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
  )
endif ()
message(STATUS "${PROJECT_NAME} version: ${VERSION_SIMPLE}")


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include (DetectArch)
target_architecture(ARCH)
if (NOT ARCH MATCHES "x86_64")
  message(FATAL_ERROR "CAN ONLY BUILD ON x86_64 ARCHITECTURE")
endif ()

set(CXX_EXECUTABLE_SRC
  src/main.cxx
  src/sdl_window.cxx
  src/tos_aot.cxx
  src/cpp2holyc.cxx
  src/holyc_routines.cxx
  src/tosprint.cxx
  src/vfs.cxx
  src/backtrace.cxx
)

set(C_EXECUTABLE_SRC
  src/sound.c
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
    CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  list(APPEND CXX_EXECUTABLE_SRC
    src/alloc_unix.cxx
    src/dbg_unix.cxx
    src/seth_unix.cxx
  )
elseif (WIN32)
  list(APPEND CXX_EXECUTABLE_SRC
    src/alloc_win32.cxx
    src/dbg_win32.cxx
    src/seth_win32.cxx
  )
else ()
  message(FATAL_ERROR "Unsupported system")
endif ()

set_source_files_properties(${C_EXECUTABLE_SRC}
  PROPERTIES
    LANGUAGE C
    COMPILE_FLAGS "-Werror=implicit-function-declaration -Wpedantic"
)


add_executable(${PROJECT_NAME}
  ${CXX_EXECUTABLE_SRC}
  ${C_EXECUTABLE_SRC}
)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set_target_properties(${PROJECT_NAME}
    PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION ON
  )
endif ()

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS YES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
    LINKER_LANGUAGE CXX
    CXX_VISIBILITY_PRESET hidden
    C_VISIBILITY_PRESET hidden
)

target_compile_features(${PROJECT_NAME}
  PRIVATE
    cxx_attributes
    cxx_alignas
    cxx_thread_local
    cxx_range_for
    cxx_auto_type
    cxx_nullptr
    cxx_nonstatic_member_init
    cxx_noexcept
    cxx_constexpr
    cxx_relaxed_constexpr
    cxx_rvalue_references
    cxx_uniform_initialization
    cxx_generalized_initializers
    cxx_trailing_return_types
    cxx_static_assert
    cxx_strong_enums
    cxx_lambdas
    cxx_generic_lambdas
    cxx_aggregate_default_initializers
    cxx_default_function_template_args
    c_function_prototypes
)

target_compile_options(${PROJECT_NAME}
  PRIVATE
    -m64
    -Wall
    -Wextra
    -Werror=shadow
    -Wno-frame-address
    -Wno-unused-command-line-argument
    -Wno-ignored-optimization-argument
    -fno-math-errno
    -fno-trapping-math
    -fno-exceptions
    -fno-omit-frame-pointer
    -fno-signaling-nans
    -fno-stack-protector
    -fno-unwind-tables
    -fno-asynchronous-unwind-tables
    -fcf-protection=none
    -mno-shstk
    -march=native
)

# libm
target_link_libraries(${PROJECT_NAME} PRIVATE m)

# sdl2 dep
find_package(SDL2 REQUIRED)
if (NOT SDL2_FOUND)
  message(FATAL_ERROR "INSTALL SDL2")
endif ()
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

add_subdirectory(asm)
add_subdirectory(vendor)

target_precompile_headers(${PROJECT_NAME}
  PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hxx>"
    "$<$<COMPILE_LANGUAGE:C>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>"
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
    CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  # uses this because cmake cant detect builtins
  include (CheckCXXSourceCompiles)
  check_cxx_source_compiles(
    "#include <stdint.h>
     #include <inttypes.h>
     int main() {
       uint32_t i = 1, j = 1;
       __atomic_compare_exchange_n(
         &i, &j, UINT32_C(0), false,
         __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST
      );
      return !(i == 0 && j == 1);
    }" HAS_ATOMIC_LOAD_N)
  if (NOT HAS_ATOMIC_LOAD_N)
    message(FATAL_ERROR "No builtin atomics, wtf are you using as your compiler?")
  endif ()
  find_package(Threads REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
elseif (WIN32)
  # bundle your own dlls when you want to package it
  # because windows doesnt like it when i statically link sdl2(stdin gets borked)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    ws2_32 synchronization shlwapi shcore dbghelp winmm
    -static # doesn't link SDL2 statically(SDL2::SDL2-static), but other libs are
  )
  # We don't need winpthreads
endif ()


if (BUILD_HCRT)
  add_custom_target(HCRT
    ALL
    COMMAND tine -ct T -f HCRT_BOOTSTRAP.BIN BuildHCRT.HC
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
endif ()

# vim: set expandtab ts=2 sw=2 :
