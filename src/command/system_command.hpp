#pragma once
#include "command_system.hpp"

class ExitHandler : public CommandHandler {
 public:
  std::string execute(const ParamMap& params) override;
};

// class CleanHandler : public CommandHandler {
//  public:
//   std::string execute(const ParamMap& params) override;
// };