#include "Types.hpp"

namespace Types {

bool isModuleIdentifier(Identifier identifier) {
    IdentifierUnion identifierUnion{};
    identifierUnion.identifier = identifier;
    return identifierUnion.bytes[3] == ModuleIdentifierCode;
}

ModuleIdentifier toModuleIdentifier(Identifier identifier) {
    IdentifierUnion identifierUnion{};
    identifierUnion.identifier = identifier;
    if (identifierUnion.bytes[3] != 0) {
        identifierUnion.identifier = -1;
    } else {
        identifierUnion.identifier = identifier;
        identifierUnion.bytes[3] = ModuleIdentifierCode;
    }
    return identifierUnion.identifier;
}

bool canBeModuleIdentifier(Identifier identifier) {
    bool canBeIdentifier{true};
    IdentifierUnion identifierUnion{};
    identifierUnion.identifier = identifier;
    if (identifierUnion.bytes[3] = 0) {
        canBeIdentifier = false;
    }
    return canBeIdentifier;
}

bool canBeServiceIdentifier(Identifier identifier) {
    IdentifierUnion identifierUnion{};
    identifierUnion.identifier = identifier;
    return identifierUnion.bytes[3] == ServiceIdentifierCode;
}

bool isServiceIdentifier(Identifier identifier) {
    IdentifierUnion identifierUnion{};
    identifierUnion.identifier = identifier;
    return identifierUnion.bytes[3] == ServiceIdentifierCode;
}

ServiceIdentifier toServiceIdentifier(Identifier identifier) {
    IdentifierUnion identifierUnion{};
    identifierUnion.identifier = identifier;
    if (identifierUnion.bytes[3] != 0) {
        identifierUnion.identifier = -1;
    } else {
        identifierUnion.identifier = identifier;
        identifierUnion.bytes[3] = ServiceIdentifierCode;
    }
    return identifierUnion.identifier;
}

} // namespace Types