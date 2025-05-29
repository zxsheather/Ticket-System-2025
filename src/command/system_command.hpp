#pragma once
#include "command_system.hpp"

class ExitHandler : public CommandHandler {
 public:
  void execute(const ParamMap& params, const std::string& timestamp) override;
};

class CleanHandler : public CommandHandler {
 public:
  void execute(const ParamMap& params, const std::string& timestamp) override;
};