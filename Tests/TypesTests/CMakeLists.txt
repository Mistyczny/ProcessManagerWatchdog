project(TypesTests)

set(TypesSource ${SOURCE_CODE}/Types.cpp)

add_executable(ModuleTypeTest ./ModuleTypeTest.cpp ${TypesSource})
target_link_libraries(ModuleTypeTest PRIVATE catchTestMain)
target_include_directories(ModuleTypeTest PRIVATE ${SOURCE_INCLUDE})

add_test(NAME ModuleTypeTest COMMAND ModuleTypeTest)