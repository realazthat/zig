# Copyright (c) 2014 Andrew Kelley
# This file is MIT licensed.
# See http://opensource.org/licenses/MIT

# LLVM_FOUND
# LLVM_INCLUDE_DIRS
# LLVM_LIBRARIES
# LLVM_LIBDIRS

find_path(LLVM_BASE_INCLUDE_DIR NAMES llvm/IR/IRBuilder.h PATHS /usr/include/llvm-3.7/)
find_path(LLVM_C_INCLUDE_DIR NAMES llvm-c/Core.h PATHS /usr/include/llvm-c-3.7/)
set(LLVM_INCLUDE_DIRS ${LLVM_C_INCLUDE_DIR} ${LLVM_BASE_INCLUDE_DIR}
      CACHE PATH "List of directories to add to the include path for LLVM")

find_program(LLVM_CONFIG_EXE NAMES llvm-config llvm-config-3.7 DOC "Path to llvm-config for LLVM")


if(NOT LLVM_LIBRARIES)
  execute_process(
      COMMAND ${LLVM_CONFIG_EXE} --libs
      OUTPUT_VARIABLE LLVM_LIBRARIES_COMPUTED
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_LIBRARIES "${LLVM_LIBRARIES_COMPUTED}" CACHE STRING "List of LLVM libraries to use when linking against LLVM")
endif()

if(NOT LLVM_SYSTEM_LIBS)
  execute_process(
      COMMAND ${LLVM_CONFIG_EXE} --system-libs
      OUTPUT_VARIABLE LLVM_SYSTEM_LIBS_COMPUTED
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_SYSTEM_LIBS "${LLVM_SYSTEM_LIBS_COMPUTED}" CACHE STRING "List of system libraries to use when linking against LLVM")
endif()

if(NOT LLVM_LIBDIRS)
  execute_process(
      COMMAND ${LLVM_CONFIG_EXE} --libdir
      OUTPUT_VARIABLE LLVM_LIBDIRS_COMPUTED
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_LIBDIRS "${LLVM_LIBDIRS_COMPUTED}" CACHE PATH "List of directories where the LLVM libraries live")
endif()



include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LLVM DEFAULT_MSG LLVM_LIBRARIES LLVM_INCLUDE_DIRS LLVM_LIBDIRS)

mark_as_advanced(LLVM_INCLUDE_DIRS LLVM_LIBRARIES LLVM_LIBDIRS)
