project(CommunicationProtocols)

include(ExternalProject)

set(ArtifactoryProtocolsPath /process-manager-local/CommunicationProtocols)

set(WatchdogServiceProtocolVersion 1.0)
set(WatchdogServiceProtocolPackage WatchdogServiceProtocol_${WatchdogServiceProtocolVersion}.tar)

set(WatchdogServiceProtocolDir ${CMAKE_CURRENT_BINARY_DIR}/watchdog-service-protocol)
ExternalProject_Add(WatchdogServiceProtocolDownload
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}/download
    BINARY_DIR
        ${WatchdogServiceProtocolDir}
    DOWNLOAD_COMMAND
        ${artifactoryDownloader} ${WatchdogServiceProtocolPackage} . ${ArtifactoryProtocolsPath}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND
        tar xf ${CMAKE_CURRENT_BINARY_DIR}/download/src/${WatchdogServiceProtocolPackage}
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

include_directories(${PROTOBUF_INCLUDE_DIR})

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER ${WatchdogServiceProtocolDir}/WatchdogService.proto)
add_library(WatchdogServiceProto ${PROTO_HEADER} ${PROTO_SRC})
add_dependencies(WatchdogServiceProto WatchdogServiceProtocolDownload)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER WatchdogModule.proto)
add_library(WatchdogModuleProto ${PROTO_HEADER} ${PROTO_SRC})