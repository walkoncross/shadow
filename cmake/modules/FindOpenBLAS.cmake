set(OpenBLAS_ROOT_DIR
    ./third_party/openblas
    /usr
    /usr/local)

find_path(OpenBLAS_INCLUDE_DIRS
          NAMES cblas.h
          PATHS ${OpenBLAS_ROOT_DIR}
          PATH_SUFFIXES include include/x86_64 include/x64
          DOC "OpenBLAS include header cblas.h"
          NO_DEFAULT_PATH)

find_library(OpenBLAS_LIBRARIES
             NAMES openblas libopenblas.dll
             PATHS ${OpenBLAS_ROOT_DIR}
             PATH_SUFFIXES lib lib64 lib/x86_64 lib/x64 lib/x86
             DOC "OpenBLAS library"
             NO_DEFAULT_PATH)

set(OpenBLAS_FOUND ON)

if (NOT OpenBLAS_INCLUDE_DIRS)
  set(OpenBLAS_FOUND OFF)
  if (NOT OpenBLAS_FIND_QUIETLY)
    message(STATUS "Could not find OpenBLAS include. Turning OpenBLAS_FOUND off")
  endif ()
endif ()

if (NOT OpenBLAS_LIBRARIES)
  set(OpenBLAS_FOUND OFF)
  if (NOT OpenBLAS_FIND_QUIETLY)
    message(STATUS "Could not find OpenBLAS lib. Turning OpenBLAS_FOUND off")
  endif ()
endif ()

if (OpenBLAS_FOUND)
  if (NOT OpenBLAS_FIND_QUIETLY)
    message(STATUS "Found OpenBLAS include: ${OpenBLAS_INCLUDE_DIRS}")
    message(STATUS "Found OpenBLAS libraries: ${OpenBLAS_LIBRARIES}")
  endif ()
  mark_as_advanced(OpenBLAS_INCLUDE_DIRS OpenBLAS_LIBRARIES OpenBLAS)
else ()
  if (OpenBLAS_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find OpenBLAS")
  endif ()
endif ()
