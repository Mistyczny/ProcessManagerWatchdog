project(CommunicationProtocols)

include_directories(${PROTOBUF_INCLUDE_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER WatchdogModule.proto)
add_library(WatchdogModuleProto ${PROTO_HEADER} ${PROTO_SRC})

PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER WatchdogService.proto)
add_library(WatchdogServiceProto ${PROTO_HEADER} ${PROTO_SRC})