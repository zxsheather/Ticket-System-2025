// Include command of Users
#pragma once
#include "../controller/user_manager.hpp"
#include "command_system.hpp"

class LoginHandler : public CommandHandler {
 private:
  UserManager& user_manager;

 public:
  LoginHandler(UserManager& manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};

class AddUserHandler : public CommandHandler {
 private:
  UserManager& user_manager;

 public:
  AddUserHandler(UserManager& manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};

class LogoutHandler : public CommandHandler {
 private:
  UserManager& user_manager;

 public:
  LogoutHandler(UserManager& manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};

class QueryProfileHandler : public CommandHandler {
 private:
  UserManager& user_manager;

 public:
  QueryProfileHandler(UserManager& manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};

class ModifyProfileHandler : public CommandHandler {
 private:
  UserManager& user_manager;

 public:
  ModifyProfileHandler(UserManager& manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};
