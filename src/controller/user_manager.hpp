#pragma once
#include <cstdint>

#include "../model/user.hpp"
#include "../stl/map.hpp"
#include "../stl/utility.hpp"
#include "../storage/bplus_tree.hpp"

class UserManager {
 private:
  BPT<uint64_t, User> user_db;

  sjtu::map<std::string, int> logged_in_users{};  // from username to privilege

  bool is_first_user{false};

 public:
  UserManager();

  int addUser(const std::string& cur_username, const std::string& username,
              const std::string& password, const std::string& name,
              const std::string& mail_addr, const int& privilege);

  int login(const std::string& username, const std::string& password);

  int logout(const std::string& username);

  sjtu::pair<int, UserProfile> queryProfile(const std::string& cur_username,
                                            const std::string& username);

  sjtu::pair<int, UserProfile> modifyProfile(const std::string& cur_username,
                                             const std::string& username,
                                             const std::string& password,
                                             const std::string& name,
                                             const std::string& mail_addr,
                                             const int& privilege);
  int isLoggedIn(const std::string& username) {
    auto iter = logged_in_users.find(username);
    if (iter == logged_in_users.end()) {
      return -1;
    }
    return iter->second;
  }
};
