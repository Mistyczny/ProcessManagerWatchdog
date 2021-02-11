#include "MongoUsersCollection.hpp"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

namespace Mongo {

UsersCollection::UsersCollection(mongocxx::client& client, std::string collectionName)
    : usersCollection{client["ProcessManager"][collectionName]} {}

bool UsersCollection::insertNewUser(const User& user) {
    bool userInserted{false};
    auto builder = document{};
    auto newModule = builder << "login" << user.getLogin()                // To prevent line move by clang
                             << "password" << user.getEncryptedPassword() // To prevent line move by clang
                             << finalize;
    auto result = usersCollection.insert_one(std::move(newModule));
    if (result) {
        userInserted = true;
    }
    return userInserted;
}

bool UsersCollection::verifyUser(const User& user) {
    bool verified{false};
    auto builder = document{};
    auto newModule = builder << "login" << user.getLogin()                // To prevent line move by clang
                             << "password" << user.getEncryptedPassword() // To prevent line move by clang
                             << finalize;
    auto result = usersCollection.find_one(std::move(newModule));
    if (result) {
        verified = true;
    }
    return verified;
}

} // namespace Mongo