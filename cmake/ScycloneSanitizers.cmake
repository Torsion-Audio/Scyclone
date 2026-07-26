# ScycloneSanitizers.cmake - per-OS sanitizer presets (policy aligned with Clang / MSVC docs).
# Presets: NONE | ASAN | ASAN_UBSAN | THREAD | MEMORY | LEAK
# One sanitizer family per build; never combine ASan, TSan, MSan in one link.

set(SCYCLONE_SANITIZERS NONE CACHE STRING "Sanitizer preset: NONE, ASAN, ASAN_UBSAN, THREAD, MEMORY, LEAK")
set_property(CACHE SCYCLONE_SANITIZERS PROPERTY STRINGS NONE ASAN ASAN_UBSAN THREAD MEMORY LEAK)

option(SCYCLONE_MSAN_TRACK_ORIGINS "Add -fsanitize-memory-track-origins=2 for MSan (higher overhead)" OFF)

option(SCYCLONE_SANITIZER_STUB_ONNX
    "Skip linking prebuilt ONNX Runtime; compile with SCYCLONE_INFERENCE_STUB (MSan / MSVC ASan / LEAK)"
    OFF)

# Prebuilt ONNX is not MSan-instrumented; MSVC ASan needs matching STL annotations;
# open-source Clang on macOS cannot link prebuilt ORT (Homebrew CI / LEAK preset).
# TSan requires every thread's code to be instrumented — the uninstrumented ORT
# threadpool produces guaranteed false positives once inference runs.
set(_scyclone_stub_onnx_required OFF)
if(SCYCLONE_SANITIZERS STREQUAL "MEMORY"
    OR SCYCLONE_SANITIZERS STREQUAL "LEAK"
    OR SCYCLONE_SANITIZERS STREQUAL "THREAD"
    OR (SCYCLONE_SANITIZERS STREQUAL "ASAN" AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
  set(_scyclone_stub_onnx_required ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin"
    AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
    AND (SCYCLONE_SANITIZERS STREQUAL "ASAN" OR SCYCLONE_SANITIZERS STREQUAL "ASAN_UBSAN"))
  set(_scyclone_stub_onnx_required ON)
endif()

if(_scyclone_stub_onnx_required)
  set(SCYCLONE_SANITIZER_STUB_ONNX ON CACHE BOOL
      "Skip linking prebuilt ONNX Runtime; compile with SCYCLONE_INFERENCE_STUB (MSan / MSVC ASan / LEAK)" FORCE)
else()
  set(SCYCLONE_SANITIZER_STUB_ONNX OFF CACHE BOOL
      "Skip linking prebuilt ONNX Runtime; compile with SCYCLONE_INFERENCE_STUB (MSan / MSVC ASan / LEAK)" FORCE)
endif()

# Linux ASAN_UBSAN: prebuilt ORT triggers UBSan vptr false positives in PluginIntegrationTest (macOS passes).
if(SCYCLONE_SANITIZERS STREQUAL "ASAN_UBSAN" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST ON CACHE BOOL
      "Skip PluginIntegrationTest (prebuilt ORT + Linux UBSan)" FORCE)
else()
  set(SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST OFF CACHE BOOL
      "Skip PluginIntegrationTest (prebuilt ORT + Linux UBSan)" FORCE)
endif()

# MSan: the DSP chain (RNBO export, SIMD paths) is not yet MSan-clean; plugin-level
# probes report uninitialised-value taint. Unit-level tests remain covered.
if(SCYCLONE_SANITIZERS STREQUAL "MEMORY")
  set(SCYCLONE_SKIP_AUTOMATION_STABILITY_TEST ON CACHE BOOL
      "Skip AutomationStabilityTest (DSP chain not MSan-clean yet)" FORCE)
else()
  set(SCYCLONE_SKIP_AUTOMATION_STABILITY_TEST OFF CACHE BOOL
      "Skip AutomationStabilityTest (DSP chain not MSan-clean yet)" FORCE)
endif()

# Linux MSan: distro libc++.so is not instrumented; link against a prefix built with -fsanitize=memory
# (see sanitize-msan-linux job in .github/workflows/sanitizers-advisory.yml).
set(SCYCLONE_MSAN_LIBCXX_PREFIX "" CACHE PATH
    "Install prefix of MSan-instrumented libc++/libc++abi (include/c++/v1, lib/libc++.so)")

if(SCYCLONE_SANITIZERS STREQUAL "NONE")
  return()
endif()

# --- Compiler/OS detection ---
set(_scyclone_is_msvc FALSE)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(_scyclone_is_msvc TRUE)
endif()

set(_scyclone_is_clang FALSE)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(_scyclone_is_clang TRUE)
endif()

set(_scyclone_is_gnu FALSE)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(_scyclone_is_gnu TRUE)
endif()

set(_scyclone_is_linux FALSE)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(_scyclone_is_linux TRUE)
endif()

set(_scyclone_is_windows FALSE)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(_scyclone_is_windows TRUE)
endif()

# --- FATAL_ERROR checks (policy: no overpromising) ---
if(SCYCLONE_SANITIZERS STREQUAL "ASAN_UBSAN")
  if(_scyclone_is_msvc)
    message(FATAL_ERROR
      "ASAN_UBSAN is not supported with MSVC. Use ASAN on Windows (-DSCYCLONE_SANITIZERS=ASAN), "
      "or use Clang on Windows and validate before enabling.")
  endif()
endif()

if(SCYCLONE_SANITIZERS STREQUAL "THREAD")
  if(_scyclone_is_windows)
    message(FATAL_ERROR
      "THREAD (TSan) is not supported on Windows in this project policy. "
      "Use Linux or macOS CI / local builds.")
  endif()
  if(_scyclone_is_msvc)
    message(FATAL_ERROR
      "THREAD (TSan) is not supported with MSVC. Use Linux or macOS.")
  endif()
endif()

if(SCYCLONE_SANITIZERS STREQUAL "MEMORY")
  if(NOT _scyclone_is_linux OR NOT _scyclone_is_clang)
    message(FATAL_ERROR
      "MEMORY (MSan) is only supported with Clang on Linux. "
      "Use the Linux CI job or WSL2.")
  endif()
  if(_scyclone_is_gnu)
    message(FATAL_ERROR
      "MEMORY (MSan) is Clang-only in this repo. Use -DCMAKE_CXX_COMPILER=clang++.")
  endif()
endif()

if(SCYCLONE_SANITIZERS STREQUAL "LEAK")
  if(NOT _scyclone_is_clang)
    message(FATAL_ERROR
      "LEAK (LeakSanitizer) requires Clang. Use open-source Clang (e.g., Homebrew LLVM) on macOS, "
      "or system Clang on Linux.")
  endif()
  if(_scyclone_is_msvc)
    message(FATAL_ERROR
      "LEAK (LeakSanitizer) is not supported with MSVC. Use Clang on macOS or Linux.")
  endif()
  if(_scyclone_is_gnu)
    message(FATAL_ERROR
      "LEAK (LeakSanitizer) is Clang-only. Use -DCMAKE_CXX_COMPILER=clang++.")
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(WARNING
      "Standalone LSan (SCYCLONE_SANITIZERS=LEAK) is not used in CI for GUI/JUCE tests "
      "(llvm/llvm-project#117476). Prefer Linux ASan with ASAN_OPTIONS=detect_leaks=1, "
      "sanitize-asan-leaks-macos in CI, or manual Xcode Instruments / leaks. "
      "LEAK preset is retained for local experiments only.")
  endif()
endif()

# --- Interface target: flags applied to library and all test executables ---
add_library(scyclone_sanitizer_flags INTERFACE)

if(_scyclone_is_msvc)
  # MSVC: only ASAN is supported in policy; ASAN_UBSAN/THREAD/MEMORY already rejected above.
  # Note: /fsanitize=address is compiler-only for MSVC; linker flag causes LNK4044 warning.
  if(SCYCLONE_SANITIZERS STREQUAL "ASAN")
    target_compile_options(scyclone_sanitizer_flags INTERFACE /fsanitize=address)
  endif()
else()
  # GCC / Clang (Linux, macOS): common flags for all sanitizer presets
  target_compile_options(scyclone_sanitizer_flags INTERFACE
    -g
    -fno-omit-frame-pointer
    -O1
  )
  if(SCYCLONE_SANITIZERS STREQUAL "ASAN")
    target_compile_options(scyclone_sanitizer_flags INTERFACE -fsanitize=address)
    target_link_options(scyclone_sanitizer_flags INTERFACE -fsanitize=address)
  elseif(SCYCLONE_SANITIZERS STREQUAL "ASAN_UBSAN")
    target_compile_options(scyclone_sanitizer_flags INTERFACE -fsanitize=address,undefined)
    target_link_options(scyclone_sanitizer_flags INTERFACE -fsanitize=address,undefined)
  elseif(SCYCLONE_SANITIZERS STREQUAL "THREAD")
    target_compile_options(scyclone_sanitizer_flags INTERFACE -fsanitize=thread)
    target_link_options(scyclone_sanitizer_flags INTERFACE -fsanitize=thread)
  elseif(SCYCLONE_SANITIZERS STREQUAL "MEMORY")
    target_compile_options(scyclone_sanitizer_flags INTERFACE -fsanitize=memory)
    target_link_options(scyclone_sanitizer_flags INTERFACE -fsanitize=memory)
    if(_scyclone_is_linux AND _scyclone_is_clang)
      target_compile_options(scyclone_sanitizer_flags INTERFACE -stdlib=libc++)
      target_link_options(scyclone_sanitizer_flags INTERFACE -stdlib=libc++)
      if(SCYCLONE_MSAN_LIBCXX_PREFIX)
        set(_scyclone_msan_libcxx "${SCYCLONE_MSAN_LIBCXX_PREFIX}")
        cmake_path(NORMAL_PATH _scyclone_msan_libcxx)
        if(NOT EXISTS "${_scyclone_msan_libcxx}/include/c++/v1")
          message(FATAL_ERROR
            "SCYCLONE_MSAN_LIBCXX_PREFIX is set but ${_scyclone_msan_libcxx}/include/c++/v1 not found. "
            "Build/install MSan libc++ there (see CI sanitize-msan-linux job).")
        endif()
        target_compile_options(scyclone_sanitizer_flags INTERFACE
          -nostdinc++
          "-isystem${_scyclone_msan_libcxx}/include/c++/v1"
        )
        target_link_options(scyclone_sanitizer_flags INTERFACE
          "-L${_scyclone_msan_libcxx}/lib"
          "-Wl,-rpath,${_scyclone_msan_libcxx}/lib"
        )
      else()
        message(WARNING
          "Linux MSan: SCYCLONE_MSAN_LIBCXX_PREFIX is empty; distro libc++.so is not MSan-instrumented "
          "(false positives likely). CI builds a prefix - see sanitize-msan-linux in .github/workflows/sanitizers-advisory.yml.")
      endif()
    endif()
    if(SCYCLONE_MSAN_TRACK_ORIGINS)
      target_compile_options(scyclone_sanitizer_flags INTERFACE -fsanitize-memory-track-origins=2)
      target_link_options(scyclone_sanitizer_flags INTERFACE -fsanitize-memory-track-origins=2)
    endif()
  elseif(SCYCLONE_SANITIZERS STREQUAL "LEAK")
    target_compile_options(scyclone_sanitizer_flags INTERFACE -fsanitize=leak)
    target_link_options(scyclone_sanitizer_flags INTERFACE -fsanitize=leak)
  endif()
endif()

# MSan: every .o in the link must be instrumented. Apply C flags to bundled C subdirs (e.g. libsamplerate).
if(SCYCLONE_SANITIZERS STREQUAL "MEMORY" AND _scyclone_is_linux AND _scyclone_is_clang)
  add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-fsanitize=memory>
    $<$<COMPILE_LANGUAGE:C>:-g>
    $<$<COMPILE_LANGUAGE:C>:-O1>
    $<$<COMPILE_LANGUAGE:C>:-fno-omit-frame-pointer>
  )
  add_link_options(-fsanitize=memory)
endif()
