set(Shadow_LINKER_LIBS "")

include(cmake/ProtoBuf.cmake)
list(APPEND Shadow_LINKER_LIBS proto)

if (${USE_GLog} STREQUAL "ON")
  list(APPEND Shadow_LINKER_LIBS glog)
  add_definitions(-DUSE_GLog)
endif ()

if (${USE_OpenCV} STREQUAL "ON")
  find_package(OpenCV PATHS ${OpenCV_ROOT} NO_DEFAULT_PATH QUIET COMPONENTS core highgui imgproc imgcodecs videoio)
  if (NOT OpenCV_FOUND) # if not OpenCV 3.x, then try to find OpenCV 2.x in default path
    find_package(OpenCV REQUIRED COMPONENTS core highgui imgproc)
  endif ()
  include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})
  list(APPEND Shadow_LINKER_LIBS ${OpenCV_LIBS})
  message(STATUS "Found OpenCV: ${OpenCV_CONFIG_PATH} (found version ${OpenCV_VERSION})")
  add_definitions(-DUSE_OpenCV)
endif ()

if (${USE_CUDA} STREQUAL "ON")
  find_package(CUDA QUIET)
  if (CUDA_FOUND)
    set(CUDA_PROPAGATE_HOST_FLAGS OFF)
    set(CUDA_NVCC_FLAGS "-gencode arch=compute_30,code=\"compute_30,sm_30\" -std=c++11")
    include_directories(SYSTEM ${CUDA_TOOLKIT_INCLUDE})
    list(APPEND Shadow_LINKER_LIBS ${CUDA_CUDART_LIBRARY} ${CUDA_cublas_LIBRARY})
    message(STATUS "Found CUDA: ${CUDA_TOOLKIT_ROOT_DIR} (found version ${CUDA_VERSION})")
    add_definitions(-DUSE_CUDA)
  else ()
    message(WARNING "Could not find CUDA, using CPU")
  endif ()
elseif (${USE_CL} STREQUAL "ON")
  find_package(OpenCL QUIET)
  if (OpenCL_FOUND)
    include_directories(SYSTEM "external/EasyCL/dist/include/easycl")
    include_directories(SYSTEM ${OpenCL_INCLUDE_DIRS})
    list(APPEND Shadow_LINKER_LIBS ${OpenCL_LIBRARIES} clew EasyCL clBLAS)
    message(STATUS "Found OpenCL: ${OpenCL_LIBRARIES} (found version ${OpenCL_VERSION_STRING})")
    add_definitions(-DUSE_CL)
  else ()
    message(WARNING "Could not find OpenCL, using CPU")
  endif ()
endif ()

if ((NOT CUDA_FOUND) AND (NOT OpenCL_FOUND))
  if (${USE_BLAS} STREQUAL "ON")
    if (${BLAS} STREQUAL "Open" OR ${BLAS} STREQUAL "open")
      find_package(OpenBLAS REQUIRED)
      include_directories(SYSTEM ${OpenBLAS_INCLUDE_DIRS})
      list(APPEND Shadow_LINKER_LIBS ${OpenBLAS_LIBRARIES} pthread)
      add_definitions(-DUSE_OpenBLAS)
    endif ()
  endif ()
endif ()
