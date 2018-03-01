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

if (OpenCL_FOUND)
  if (APPLE)
    set(CL_HEADER_FILE "${OpenCL_INCLUDE_DIRS}/OpenCL/cl.h")
  else ()
    set(CL_HEADER_FILE "${OpenCL_INCLUDE_DIRS}/CL/cl.h")
  endif ()
  parse_header(${CL_HEADER_FILE}
               CL_VERSION_2_0 CL_VERSION_1_2 CL_VERSION_1_1 CL_VERSION_1_0)
  if (CL_VERSION_2_0 EQUAL 1)
    set(OpenCL_VERSION_STRING "2.0")
  elseif (CL_VERSION_1_2 EQUAL 1)
    set(OpenCL_VERSION_STRING "1.2")
  elseif (CL_VERSION_1_1 EQUAL 1)
    set(OpenCL_VERSION_STRING "1.1")
  elseif (CL_VERSION_1_0 EQUAL 1)
    set(OpenCL_VERSION_STRING "1.0")
  else ()
    set(OpenCL_VERSION_STRING "?")
  endif ()
  if (NOT OpenCL_FIND_QUIETLY)
    message(STATUS "Found OpenCL: ${OpenCL_INCLUDE_DIRS}, ${OpenCL_LIBRARIES} (found version ${OpenCL_VERSION_STRING})")
  endif ()
  mark_as_advanced(OpenCL_ROOT_DIR OpenCL_INCLUDE_DIRS OpenCL_LIBRARIES)
else ()
  if (OpenCL_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find OpenCL")
  endif ()
endif ()
