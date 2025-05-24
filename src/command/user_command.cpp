
#include "user_command.hpp"

#include "../controller/user_manager.hpp"

LoginHandler::LoginHandler(UserManager& manager) : user_manager(manager) {}

std::string LoginHandler::execute(const ParamMap& params) {
  std::string username = params.get('u');
  std::string password = params.get('p');
  int result = user_manager.login(username, password);
  return result == 0 ? "0" : "-1";
}

AddUserHandler::AddUserHandler(UserManager& manager) : user_manager(manager) {}

std::string AddUserHandler::execute(const ParamMap& params) {
  std::string cur_username = params.has('c') ? params.get('c') : "";
  std::string username = params.get('u');
  std::string password = params.get('p');
  std::string name = params.get('n');
  std::string mail_addr = params.get('m');
  int privilege = params.has('g') ? std::stoi(params.get('g')) : -1;
  int result = user_manager.addUser(cur_username, username, password, name,
                                    mail_addr, privilege);
  return result == 0 ? "0" : "-1";
}

LogoutHandler::LogoutHandler(UserManager& manager) : user_manager(manager) {}

std::string LogoutHandler::execute(const ParamMap& params) {
  std::string username = params.get('u');
  int result = user_manager.logout(username);
  return result == 0 ? "0" : "-1";
}

QueryProfileHandler::QueryProfileHandler(UserManager& manager)
    : user_manager(manager) {}
std::string QueryProfileHandler::execute(const ParamMap& params) {
  std::string cur_username = params.get('c');
  std::string username = params.get('u');
  auto result = user_manager.queryProfile(cur_username, username);
  if (result.first == -1) {
    return "-1";
  } else {
    return result.second.username + " " + result.second.name + " " +
           result.second.mail_addr + " " +
           std::to_string(result.second.privilege);
  }
}

ModifyProfileHandler::ModifyProfileHandler(UserManager& manager)
    : user_manager(manager) {}
std::string ModifyProfileHandler::execute(const ParamMap& params) {
  std::string cur_username = params.get('c');
  std::string username = params.get('u');
  std::string password = params.has('p') ? params.get('p') : "";
  std::string name = params.has('n') ? params.get('n') : "";
  std::string mail_addr = params.has('m') ? params.get('m') : "";
  int privilege = params.has('g') ? std::stoi(params.get('g')) : -1;
  auto result = user_manager.modifyProfile(cur_username, username, password,
                                           name, mail_addr, privilege);
  if (result.first == -1) {
    return "-1";
  } else {
    return result.second.username + " " + result.second.name + " " +
           result.second.mail_addr + " " +
           std::to_string(result.second.privilege);
  }
}
