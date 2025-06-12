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

// JSON version for web interface
CommandResult LoginHandler::executeForWeb(const ParamMap& params,
                                         const std::string& timestamp) {
  std::string username = params.get('u');
  std::string password = params.get('p');
  int result = user_manager.login(username, password);
  
  if (result == 0) {
    CommandResult cmd_result(true, "Login successful");
    cmd_result.data = std::to_string(user_manager.isLoggedIn(username));
    return cmd_result;
  } else {
    return CommandResult(false, "Login failed");
  }
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

// JSON version for web interface
CommandResult AddUserHandler::executeForWeb(const ParamMap& params,
                                           const std::string& timestamp) {
  std::string cur_username = params.has('c') ? params.get('c') : "";
  std::string username = params.get('u');
  std::string password = params.get('p');
  std::string name = params.get('n');
  std::string mail_addr = params.get('m');
  int privilege = params.has('g') ? std::stoi(params.get('g')) : 1;
  
  // 如果没有当前用户名（注册新用户），设置默认权限为1
  if (cur_username.empty()) {
    privilege = 1;  // 新注册用户默认权限为1
  }
  
  int result = user_manager.addUser(cur_username, username, password, name,
                                    mail_addr, privilege);
  
  if (result == 0) {
    return CommandResult(true, "User added successfully");
  } else {
    return CommandResult(false, "Failed to add user");
  }
}

LogoutHandler::LogoutHandler(UserManager& manager) : user_manager(manager) {}

void LogoutHandler::execute(const ParamMap& params,
                            const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::string username = params.get('u');
  int result = user_manager.logout(username);
  std::cout << result << '\n';
}

// JSON version for web interface
CommandResult LogoutHandler::executeForWeb(const ParamMap& params,
                                          const std::string& timestamp) {
  std::string username = params.get('u');
  int result = user_manager.logout(username);
  
  if (result == 0) {
    return CommandResult(true, "Logout successful");
  } else {
    return CommandResult(false, "Logout failed");
  }
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

// JSON version for web interface
CommandResult QueryProfileHandler::executeForWeb(const ParamMap& params,
                                                const std::string& timestamp) {
  std::string cur_username = params.get('c');
  std::string username = params.get('u');
  auto result = user_manager.queryProfile(cur_username, username);
  
  if (result.first == -1) {
    return CommandResult(false, "Failed to query profile");
  } else {
    CommandResult cmd_result(true, "Profile query successful");
    cmd_result.data = result.second.username + " " + result.second.name + " " +
                     result.second.mail_addr + " " + std::to_string(result.second.privilege);
    return cmd_result;
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

// JSON version for web interface
CommandResult ModifyProfileHandler::executeForWeb(const ParamMap& params,
                                                 const std::string& timestamp) {
  std::string cur_username = params.get('c');
  std::string username = params.get('u');
  std::string password = params.has('p') ? params.get('p') : "";
  std::string name = params.has('n') ? params.get('n') : "";
  std::string mail_addr = params.has('m') ? params.get('m') : "";
  int privilege = params.has('g') ? std::stoi(params.get('g')) : -1;
  auto result = user_manager.modifyProfile(cur_username, username, password,
                                           name, mail_addr, privilege);
  
  if (result.first == -1) {
    return CommandResult(false, "Failed to modify profile");
  } else {
    CommandResult cmd_result(true, "Profile modified successfully");
    cmd_result.data = result.second.username + " " + result.second.name + " " +
                     result.second.mail_addr + " " + std::to_string(result.second.privilege);
    return cmd_result;
  }
}
