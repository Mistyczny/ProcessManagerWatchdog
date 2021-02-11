#pragma once
#include <mongocxx/client.hpp>

namespace Mongo {

class User {
private:
    std::string login;
    std::string password;

public:
    User(std::string login, std::string password) {
        this->login = login;
        this->password = password;
    }
    virtual ~User() = default;

    std::string getEncryptedPassword() const {
        std::string encryptedPassword{};

        for (auto rit = this->password.rbegin(); rit != this->password.rend(); ++rit) {
            encryptedPassword += *rit;
        }
        return encryptedPassword;
    }
    std::string getLogin() const { return this->login; }
    std::string getPassword() const { return this->password; }
};

class UsersCollection {
private:
    mongocxx::collection usersCollection;

public:
    UsersCollection(mongocxx::client& client, std::string collectionName);
    virtual ~UsersCollection() = default;

    bool insertNewUser(const User& user);
    bool verifyUser(const User& user);
};

} // namespace Mongo