
#include "user_manager.hpp"

#include "../utilities/hash.hpp"

UserManager::UserManager() : user_db("users") {
  if (user_db.empty()) {
    is_first_user = true;
  }
}

int UserManager::addUser(const std::string& cur_username,
                         const std::string& username,
                         const std::string& password, const std::string& name,
                         const std::string& mail_addr, const int& privilege) {
  long long username_hash = Hash::hashKey(username);
  if(is_first_user){
    User user(username, password, name, mail_addr, 10);
    user_db.insert(username_hash, user);
    is_first_user = false;
    return 0;
  }
  if(user_db.exists(username_hash)){
    return -1;
  }
  auto iter = logged_in_users.find(cur_username);
  if(iter == logged_in_users.end()){
    return -1;
  }
  int cur_privilege = iter->second;
  if(cur_privilege <= privilege){
    return -1;
  }
  User user(username, password, name, mail_addr, privilege);
  user_db.insert(username_hash, user);
  return 0;
}

int UserManager::login(const std::string& username,
                       const std::string& password) {
  if (logged_in_users.find(username) != logged_in_users.end()) {
    return -1;
  }
  long long username_hash = Hash::hashKey(username);
  sjtu::vector<User> result = user_db.find(username_hash);
  if (result.empty()) {
    return -1;
  }
  if (result[0].password.comparedToString(password) != 0) {
    return -1;
  }
  logged_in_users[username] = result[0].privilege;
  return 0;
}

int UserManager::logout(const std::string& username) {
  auto iter = logged_in_users.find(username);
  if (iter == logged_in_users.end()) {
    return -1;
  }
  logged_in_users.erase(iter);
  return 0;
}

sjtu::pair<int, UserProfile> UserManager::queryProfile(
    const std::string& cur_username, const std::string& username) {
  auto iter = logged_in_users.find(cur_username);
  if (iter == logged_in_users.end()) {
    return sjtu::pair(-1, UserProfile());
  }
  int cur_privilege = iter->second;
  sjtu::vector<User> result = user_db.find(Hash::hashKey(username));
  if (result.empty()) {
    return sjtu::pair(-1, UserProfile());
  }
  if (cur_privilege < result[0].privilege ||
      (cur_privilege == result[0].privilege && cur_username != username)) {
    return sjtu::pair(-1, UserProfile());
  }
  return sjtu::pair(
      0, UserProfile{result[0].username.toString(), result[0].name.toString(),
                     result[0].mail_addr.toString(), result[0].privilege});
}

sjtu::pair<int, UserProfile> UserManager::modifyProfile(
    const std::string& cur_username, const std::string& username,
    const std::string& password, const std::string& name,
    const std::string& mail_addr, const int& privilege) {
  auto iter = logged_in_users.find(cur_username);
  if (iter == logged_in_users.end()) {
    return sjtu::pair{-1, UserProfile()};
  }
  int cur_privilege = iter->second;
  if (cur_privilege <= privilege) {
    return sjtu::pair{-1, UserProfile()};
  }
  long long username_hash = Hash::hashKey(username);
  sjtu::vector<User> result = user_db.find(username_hash);
  if (result.empty()) {
    return sjtu::pair{-1, UserProfile()};
  }
  User user = result[0];
  if (cur_privilege < user.privilege ||
      (cur_privilege == user.privilege && cur_username != username)) {
    return sjtu::pair{-1, UserProfile()};
  }
  if (password != "") {
    user.password = password;
  }
  if (name != "") {
    user.name = name;
  }
  if (mail_addr != "") {
    user.mail_addr = mail_addr;
  }
  if (privilege != -1) {
    user.privilege = privilege;
    if(cur_username == username){
      logged_in_users[cur_username] = privilege;
    }
  }
  // need further optimization
  user_db.remove(username_hash, result[0]);
  user_db.insert(username_hash, user);

  return sjtu::pair{0, UserProfile{
    username,
    user.name.toString(),
    user.mail_addr.toString(),
    user.privilege
  }};
}
