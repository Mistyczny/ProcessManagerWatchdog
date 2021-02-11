#pragma once
#include <array>
#include <cstdint>

namespace Types {

typedef int32_t Identifier;
typedef int32_t ModuleIdentifier;
typedef int32_t ServiceIdentifier;

#define ModuleIdentifierCode 0x2A
#define ServiceIdentifierCode 0x34

union IdentifierUnion {
    std::array<int8_t, 4> bytes;
    Identifier identifier;
};

[[nodiscard]] bool canBeModuleIdentifier(Identifier identifier);
[[nodiscard]] bool isModuleIdentifier(Identifier identifier);
[[nodiscard]] ModuleIdentifier toModuleIdentifier(Identifier identifier);

[[nodiscard]] bool canBeServiceIdentifier(Identifier identifier);
[[nodiscard]] bool isServiceIdentifier(Identifier identifier);
[[nodiscard]] ServiceIdentifier toServiceIdentifier(Identifier identifier);

[[nodiscard]] constexpr ModuleIdentifier getMinimalModuleIdentifier() {
    IdentifierUnion identifierUnion{};
    identifierUnion.bytes[3] = ModuleIdentifierCode;
    return identifierUnion.identifier;
}

} // namespace Types