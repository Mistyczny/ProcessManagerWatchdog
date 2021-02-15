set(PackagingBase "${CMAKE_CURRENT_SOURCE_DIR}/Packaging")

function(ConfigurePackage packageName summary)
    set(PackageConfigDirectory "${PackagingBase}/${packageName}")

    set(CPACK_DEBIAN_${packageName}_PACKAGE_SUMMARY ${summary})
    set(CPACK_DEBIAN_${packageName}_PACKAGE_NAME "${packageName}")
    set(CPACK_DEBIAN_${packageName}_FILE_NAME "${CPACK_DEBIAN_${packageName}_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.deb")
endfunction()

function(ConfigureRpmPackage packageName)
    set(PackageConfigDirectory "${PackagingBase}/${packageName}")
    set(PackageRequestFile "${PackageConfigDirectory}/${packageName}.spec.req")

    set(CPACK_RPM_${packageName}_PACKAGE_NAME "${packageName}")
    set(CPACK_RPM_${packageName}_FILE_NAME "${CPACK_RPM_${packageName}_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.rpm")
endfunction()