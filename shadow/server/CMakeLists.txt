if (${BUILD_SERVICE} AND Protobuf_FOUND AND gRPC_FOUND)
  file(GLOB proto_files tmp *.proto)
  GenGRPC(proto_srcs proto_hdrs ${proto_files})
  add_library(grpc_proto STATIC ${proto_srcs} ${proto_hdrs})
  target_include_directories(grpc_proto PUBLIC ${gRPC_INCLUDE_DIRS})
  if (MSVC)
    target_compile_definitions(grpc_proto PUBLIC -D_WIN32_WINNT=0x0600)
    set_target_properties(grpc_proto PROPERTIES DEBUG_POSTFIX "d")
  endif ()
  install(TARGETS grpc_proto DESTINATION ${Shadow_INSTALL_LIB_PREFIX})

  file(GLOB_RECURSE tmp *_server.cpp)
  set(shadow_server_src ${shadow_server_src} ${tmp})

  file(GLOB_RECURSE tmp *_client.cpp)
  set(shadow_client_src ${shadow_client_src} ${tmp})

  set(shadow_server_src ${shadow_server_src} PARENT_SCOPE)
  set(shadow_client_src ${shadow_client_src} PARENT_SCOPE)

  set(Shadow_GRPC_PROTO_LIB grpc_proto PARENT_SCOPE)
endif ()
