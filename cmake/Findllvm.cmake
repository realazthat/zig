# Copyright (c) 2014 Andrew Kelley
# This file is MIT licensed.
# See http://opensource.org/licenses/MIT


# - Try to find LLVM
# Once done this will define
# LLVM_FOUND - found LLVM, and all necessary libraries
# LLVM_LIBRARY_DIRS - the directories where llvm libraries live
# LLVM_LIBRARIES - the libraries llvm needs to link against
# LLVM_INCLUDE_DIRS - the directories where the llvm headers live
#
# Variables used by the module, overridable on the commandline:
# LLVM_CONFIG_EXE - path to llvm-config, if specified, everything else is implicit; cmake will attempt to find this
# LLVM_BASE_INCLUDE_DIR - path(s) to llvm include directory(s)
# LLVM_C_INCLUDE_DIR - path(s) to llvm-c include directory(s)
# LLVM_LIBRARY_DIR - path(s) to llvm library directories
# LLVM_BASE_LIBRARY - list of llvm libraries to link against
# LLVM_SYSTEM_LIBRARY - list of system libraries to link against, required by llvm
#


find_program(LLVM_CONFIG_EXE NAMES llvm-config llvm-config-3.7 DOC "Path to llvm-config for LLVM; if specified, many of the other llvm-related vars are implicit")


#NOTE: TODO: find this with llvm-config??
find_path(LLVM_BASE_INCLUDE_DIR NAMES llvm/IR/IRBuilder.h PATHS /usr/include/llvm-3.7/ DOC "List of directories to add to the include path for LLVM")
find_path(LLVM_C_INCLUDE_DIR NAMES llvm-c/Core.h PATHS /usr/include/llvm-c-3.7/ DOC "List of directories to add to the include path for LLVM-C library")
mark_as_advanced(LLVM_BASE_INCLUDE_DIR LLVM_C_INCLUDE_DIR)

set(LLVM_INCLUDE_DIRS ${LLVM_C_INCLUDE_DIR} ${LLVM_BASE_INCLUDE_DIR})

#NOTE: TODO: Why do we not have a separate LLVM_C_LIBRARY? 
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

if(NOT LLVM_LIBRARY_DIR)
  execute_process(
      COMMAND ${LLVM_CONFIG_EXE} --libdir
      OUTPUT_VARIABLE LLVM_LIBDIRS_COMPUTED
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_LIBRARY_DIR "${LLVM_LIBDIRS_COMPUTED}" CACHE PATH "List of directories where the LLVM libraries live")
  mark_as_advanced(LLVM_LIBRARY_DIR)
endif()





include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LLVM DEFAULT_MSG LLVM_LIBRARY_DIR LLVM_BASE_LIBRARY LLVM_SYSTEM_LIBRARY LLVM_BASE_INCLUDE_DIR LLVM_C_INCLUDE_DIR)


if(LLVM_FOUND)
  set(LLVM_LIBRARY_DIRS ${LLVM_LIBRARY_DIR})
  set(LLVM_LIBRARIES ${LLVM_SYSTEM_LIBRARY} ${LLVM_BASE_LIBRARY})
  set(LLVM_INCLUDE_DIRS ${LLVM_INCLUDE_DIR})
endif()
