#include "user_command.hpp"

#include <iostream>

#include "../controller/user_manager.hpp"

LoginHandler::LoginHandler(UserManager& manager) : user_manager(manager) {}

void LoginHandler::execute(const ParamMap& params,
                           const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::string username = params.get('u');
  std::string password = params.get('p');
  int result = user_manager.login(username, password);
  std::cout << result << '\n';
}

AddUserHandler::AddUserHandler(UserManager& manager) : user_manager(manager) {}

void AddUserHandler::execute(const ParamMap& params,
                             const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::string cur_username = params.has('c') ? params.get('c') : "";
  std::string username = params.get('u');
  std::string password = params.get('p');
  std::string name = params.get('n');
  std::string mail_addr = params.get('m');
  int privilege = params.has('g') ? std::stoi(params.get('g')) : -1;
  int result = user_manager.addUser(cur_username, username, password, name,
                                    mail_addr, privilege);
  std::cout << result << '\n';
}

LogoutHandler::LogoutHandler(UserManager& manager) : user_manager(manager) {}

void LogoutHandler::execute(const ParamMap& params,
                            const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::string username = params.get('u');
  int result = user_manager.logout(username);
  std::cout << result << '\n';
}

QueryProfileHandler::QueryProfileHandler(UserManager& manager)
    : user_manager(manager) {}
void QueryProfileHandler::execute(const ParamMap& params,
                                  const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::string cur_username = params.get('c');
  std::string username = params.get('u');
  auto result = user_manager.queryProfile(cur_username, username);
  if (result.first == -1) {
    std::cout << "-1\n";
  } else {
    std::cout << result.second.username << " " << result.second.name << " "
              << result.second.mail_addr << " " << result.second.privilege
              << '\n';
  }
}

ModifyProfileHandler::ModifyProfileHandler(UserManager& manager)
    : user_manager(manager) {}
void ModifyProfileHandler::execute(const ParamMap& params,
                                   const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::string cur_username = params.get('c');
  std::string username = params.get('u');
  std::string password = params.has('p') ? params.get('p') : "";
  std::string name = params.has('n') ? params.get('n') : "";
  std::string mail_addr = params.has('m') ? params.get('m') : "";
  int privilege = params.has('g') ? std::stoi(params.get('g')) : -1;
  auto result = user_manager.modifyProfile(cur_username, username, password,
                                           name, mail_addr, privilege);
  if (result.first == -1) {
    std::cout << "-1\n";
  } else {
    std::cout << result.second.username << " " << result.second.name << " "
              << result.second.mail_addr << " " << result.second.privilege
              << '\n';
  }
}
