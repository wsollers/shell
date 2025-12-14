# Force Clang-18 + libc++ on Unix (Linux/macOS). Use via CMakePresets.json.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Set the compiler to clang-18 specifically for Linux/Ubuntu
if(NOT CMAKE_C_COMPILER)
  set(CMAKE_C_COMPILER clang-18)
endif()
if(NOT CMAKE_CXX_COMPILER)
  set(CMAKE_CXX_COMPILER clang++-18)
endif()

# Explicitly set libc++ flags and paths
set(_LIBCXX_FLAGS "-stdlib=libc++")

# Add explicit include paths for libc++ on Ubuntu/Linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Common libc++ header locations
  set(_LIBCXX_INCLUDE_PATHS "")
  foreach(_path 
    "/usr/include/c++/v1"
    "/usr/lib/llvm-18/include/c++/v1" 
    "/usr/include/libcxx"
    "/usr/local/include/c++/v1"
  )
    if(EXISTS ${_path})
      list(APPEND _LIBCXX_INCLUDE_PATHS "-isystem ${_path}")
      break()
    endif()
  endforeach()
  
  if(_LIBCXX_INCLUDE_PATHS)
    string(JOIN " " _LIBCXX_INCLUDE_FLAGS ${_LIBCXX_INCLUDE_PATHS})
    set(_LIBCXX_FLAGS "${_LIBCXX_FLAGS} ${_LIBCXX_INCLUDE_FLAGS}")
  endif()
endif()

set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} ${_LIBCXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} ${_LIBCXX_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${CMAKE_SHARED_LINKER_FLAGS_INIT} ${_LIBCXX_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${CMAKE_MODULE_LINKER_FLAGS_INIT} ${_LIBCXX_FLAGS}")

if(UNIX AND NOT APPLE)
  set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -lc++ -lc++abi")
endif()
