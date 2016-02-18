# Copyright (c) 2014 Andrew Kelley
# This file is MIT licensed.
# See http://opensource.org/licenses/MIT

# LLVM_FOUND
# LLVM_INCLUDE_DIRS
# LLVM_LIBRARIES
# LLVM_LIBDIRS

find_path(LLVM_BASE_INCLUDE_DIR NAMES llvm/IR/IRBuilder.h PATHS /usr/include/llvm-3.7/ DOC "List of directories to add to the include path for LLVM")
find_path(LLVM_C_INCLUDE_DIR NAMES llvm-c/Core.h PATHS /usr/include/llvm-c-3.7/ DOC "List of directories to add to the include path for LLVM C library")

set(LLVM_INCLUDE_DIR ${LLVM_C_INCLUDE_DIR} ${LLVM_BASE_INCLUDE_DIR})


find_program(LLVM_CONFIG_EXE NAMES llvm-config llvm-config-3.7 DOC "Path to llvm-config for LLVM")


if(NOT LLVM_BASE_LIBRARY)
  execute_process(
      COMMAND ${LLVM_CONFIG_EXE} --libs
      OUTPUT_VARIABLE LLVM_LIBRARIES_COMPUTED
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_BASE_LIBRARY "${LLVM_LIBRARIES_COMPUTED}" CACHE STRING "List of LLVM libraries to use when linking against LLVM")
  mark_as_advanced(LLVM_BASE_LIBRARY)
endif()

if(NOT LLVM_SYSTEM_LIBRARY)
  execute_process(
      COMMAND ${LLVM_CONFIG_EXE} --system-libs
      OUTPUT_VARIABLE LLVM_SYSTEM_LIBS_COMPUTED
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_SYSTEM_LIBRARY "${LLVM_SYSTEM_LIBS_COMPUTED}" CACHE STRING "List of system libraries to use when linking against LLVM")
  mark_as_advanced(LLVM_SYSTEM_LIBRARY)
endif()

if(NOT LLVM_LIBDIR)
  execute_process(
      COMMAND ${LLVM_CONFIG_EXE} --libdir
      OUTPUT_VARIABLE LLVM_LIBDIRS_COMPUTED
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_LIBDIR "${LLVM_LIBDIRS_COMPUTED}" CACHE PATH "List of directories where the LLVM libraries live")
  mark_as_advanced(LLVM_LIBDIR)
endif()





include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LLVM DEFAULT_MSG LLVM_LIBRARIES LLVM_INCLUDE_DIRS LLVM_LIBDIRS)

if(LLVM_FOUND)
  set(LLVM_LIBDIRS ${LLVM_LIBDIR})
  set(LLVM_LIBRARIES ${LLVM_SYSTEM_LIBRARY} ${LLVM_BASE_LIBRARY})
  set(LLVM_INCLUDE_DIRS ${LLVM_INCLUDE_DIR})
endif()
