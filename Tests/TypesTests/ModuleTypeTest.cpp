#include "Types.hpp"
#include <catch2/catch.hpp>

TEST_CASE("ModuleTypeTest", "[Types]") {
    REQUIRE(Types::isModuleIdentifier(1) == false);
    REQUIRE(Types::isModuleIdentifier(2) == false);
    REQUIRE(Types::isModuleIdentifier(3) == false);
    Types::ModuleIdentifier moduleIdentifier = Types::toModuleIdentifier(1);
    REQUIRE(Types::isModuleIdentifier(moduleIdentifier) == true);
    Types::ServiceIdentifier serviceIdentifier = Types::toServiceIdentifier(1);
    REQUIRE(Types::isModuleIdentifier(serviceIdentifier) == false);
}