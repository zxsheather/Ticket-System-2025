// Include command of Users
#pragma once
#include "../controller/user_manager.hpp"
#include "command_system.hpp"

class LoginHandler : public CommandHandler {
 private:
  UserManager& user_manager;

 public:
  LoginHandler(UserManager& manager);
  std::string execute(const ParamMap& params) override;
};

class AddUserHandler : public CommandHandler {
 private:
  UserManager& user_manager;

 public:
  AddUserHandler(UserManager& manager);
  std::string execute(const ParamMap& params) override;
};

class LogoutHandler : public CommandHandler {
 private:
  UserManager& user_manager;

 public:
  LogoutHandler(UserManager& manager);
  std::string execute(const ParamMap& params) override;
};

class QueryProfileHandler : public CommandHandler {
 private:
  UserManager& user_manager;

 public:
  QueryProfileHandler(UserManager& manager);
  std::string execute(const ParamMap& params) override;
};

class ModifyProfileHandler : public CommandHandler {
 private:
  UserManager& user_manager;

 public:
  ModifyProfileHandler(UserManager& manager);
  std::string execute(const ParamMap& params) override;
};
