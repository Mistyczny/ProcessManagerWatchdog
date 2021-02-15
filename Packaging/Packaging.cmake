include(${CMAKE_CURRENT_LIST_DIR}/CMakePackageMacros.cmake)

set(CPACK_GENERATOR "DEB")
set(VERSION "1.0.1")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Kacper Waśniowski")
set(CPACK_PACKAGE_VENDOR "Kacper Waśniowski")
set(CPACK_PACKAGE_NAME "ProcessManagerWatchdog")

function(make_package package_target)
    include(${CMAKE_CURRENT_LIST_DIR}/${package_target}/CPackConfig.cmake)
    add_custom_target(${package_target}
                        COMMAND "${CMAKE_CPACK_COMMAND}"
                        "-B" "${CMAKE_CURRENT_SOURCE_DIR}/_cpack-${package_target}"
                        "-D" "CPACK_COMPONENTS_ALL=${package_target}"
                        DEPENDS ${ARGN})
endfunction(make_package)

include(CPack)