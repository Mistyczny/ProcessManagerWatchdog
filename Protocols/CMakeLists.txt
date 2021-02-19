project(CommunicationProtocols)

include(ExternalProject)

set(ArtifactoryProtocolsPath /process-manager-local/CommunicationProtocols)

#
# Watchdog-Service protocol
#

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

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER ${WatchdogServiceProtocolDir}/WatchdogService.proto)
add_library(WatchdogServiceProto ${PROTO_HEADER} ${PROTO_SRC})
target_include_directories(WatchdogServiceProto
        PUBLIC
    ${PROTOBUF_INCLUDE_DIR}
)
add_dependencies(WatchdogServiceProto WatchdogServiceProtocolDownload)

#
# Watchdog-Module protocol
#

set(WatchdogModuleProtocolVersion 1.0)
set(WatchdogModuleProtocolPackage WatchdogModuleProtocol_${WatchdogModuleProtocolVersion}.tar)

set(WatchdogModuleProtocolDir ${CMAKE_CURRENT_BINARY_DIR}/watchdog-service-protocol)
ExternalProject_Add(WatchdogModuleProtocolDownload
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}/download
    BINARY_DIR
        ${WatchdogModuleProtocolDir}
    DOWNLOAD_COMMAND
        ${artifactoryDownloader} ${WatchdogModuleProtocolPackage} . ${ArtifactoryProtocolsPath}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND
        tar xf ${CMAKE_CURRENT_BINARY_DIR}/download/src/${WatchdogModuleProtocolPackage}
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER ${WatchdogModuleProtocolDir}/WatchdogModule.proto)
add_library(WatchdogModuleProto ${PROTO_HEADER} ${PROTO_SRC})
target_include_directories(WatchdogModuleProto
        PUBLIC
    ${PROTOBUF_INCLUDE_DIR}
)
add_dependencies(WatchdogModuleProto WatchdogModuleProtocolDownload)