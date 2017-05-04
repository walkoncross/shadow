include(FindPackageHandleStandardArgs)

set(OpenCL_ROOT_DIR "" CACHE PATH "Folder contains OpenCL")

set(OpenCL_DIR ${OpenCL_ROOT_DIR} /usr /usr/local /usr/local/cuda)

find_path(OpenCL_INCLUDE_DIRS
          NAMES OpenCL/cl.h CL/cl.h
          PATHS ${OpenCL_DIR}
          PATH_SUFFIXES include include/x86_64 include/x64
          DOC "OpenCL include header OpenCL/cl.h or CL/cl.h")

find_library(OpenCL_LIBRARIES
             NAMES OpenCL
             PATHS ${OpenCL_DIR}
             PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64 lib/x86
             DOC "OpenCL library")

find_package_handle_standard_args(OpenCL DEFAULT_MSG OpenCL_INCLUDE_DIRS OpenCL_LIBRARIES)

if (NOT OpenCL_INCLUDE_DIRS AND NOT OpenCL_FIND_QUIETLY)
  message(STATUS "Could not find OpenCL include")
endif ()

if (NOT OpenCL_LIBRARIES AND NOT OpenCL_FIND_QUIETLY)
  message(STATUS "Could not find OpenCL lib")
endif ()

include(CheckSymbolExists)

if (OpenCL_FOUND)
  if (APPLE)
    set(CL_HEADER_FILE "${OpenCL_INCLUDE_DIRS}/OpenCL/cl.h")
  else ()
    set(CL_HEADER_FILE "${OpenCL_INCLUDE_DIRS}/CL/cl.h")
  endif ()
  check_symbol_exists(CL_VERSION_2_0 ${CL_HEADER_FILE} HAVE_CL_2_0)
  if (HAVE_CL_2_0)
    set(OpenCL_VERSION_STRING "2.0")
  else ()
    check_symbol_exists(CL_VERSION_1_2 ${CL_HEADER_FILE} HAVE_CL_1_2)
    if (HAVE_CL_1_2)
      set(OpenCL_VERSION_STRING "1.2")
    else ()
      set(OpenCL_VERSION_STRING "0.0")
    endif ()
  endif ()
  if (NOT OpenCL_FIND_QUIETLY)
    message(STATUS "Found OpenCL include: ${OpenCL_INCLUDE_DIRS}")
    message(STATUS "Found OpenCL libraries: ${OpenCL_LIBRARIES}")
  endif ()
  mark_as_advanced(OpenCL_ROOT_DIR OpenCL_INCLUDE_DIRS OpenCL_LIBRARIES)
else ()
  if (OpenCL_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find OpenCL")
  endif ()
endif ()
