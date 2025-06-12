#pragma once
#include "command_system.hpp"
#include "../controller/train_manager.hpp"
#include "../controller/user_manager.hpp"
#include "../controller/order_manager.hpp"


class ExitHandler : public CommandHandler {
 public:
  void execute(const ParamMap& params, const std::string& timestamp) override;
  CommandResult executeForWeb(const ParamMap& params, const std::string& timestamp) override;
};

class CleanHandler : public CommandHandler {
 public:
  void execute(const ParamMap& params, const std::string& timestamp) override;
  CommandResult executeForWeb(const ParamMap& params, const std::string& timestamp) override;
};

class DataNumQueryHandler : public CommandHandler {
private:
  TrainManager& train_manager;
  UserManager& user_manager;
  OrderManager& order_manager;
public:
  DataNumQueryHandler(TrainManager& tm, UserManager& um, OrderManager& om);
  void execute(const ParamMap& params, const std::string& timestamp) override;
  CommandResult executeForWeb(const ParamMap& params, const std::string& timestamp) override;
};
