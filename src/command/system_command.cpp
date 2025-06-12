#include "system_command.hpp"

#include <filesystem>
#include <iostream>

void ExitHandler::execute(const ParamMap& params,
                          const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::cout << "bye";
  exit(0);
}

CommandResult ExitHandler::executeForWeb(const ParamMap& params,
                                        const std::string& timestamp) {
  CommandResult result;
  result.success = true;
  result.message = "System shutdown requested";
  result.data = "bye";
  // Note: In web context, we don't actually exit, just return success
  return result;
}

void CleanHandler::execute(const ParamMap& params,
                           const std::string& timestamp) {
  std::filesystem::remove("order.block");
  std::filesystem::remove("order.index");
  std::filesystem::remove("train.block");
  std::filesystem::remove("train.index");
  std::filesystem::remove("seat.memoryriver");
  std::filesystem::remove("station.block");
  std::filesystem::remove("station.index");
  std::filesystem::remove("route.block");
  std::filesystem::remove("route.index");
  std::filesystem::remove("pending.block");
  std::filesystem::remove("pending.index");
  std::filesystem::remove("users.block");
  std::filesystem::remove("users.index");
  std::cout << '[' << timestamp << "] 0";
}

CommandResult CleanHandler::executeForWeb(const ParamMap& params,
                                         const std::string& timestamp) {
  CommandResult result;

  try {
    std::filesystem::remove("order.block");
    std::filesystem::remove("order.index");
    std::filesystem::remove("train.block");
    std::filesystem::remove("train.index");
    std::filesystem::remove("seat.memoryriver");
    std::filesystem::remove("station.block");
    std::filesystem::remove("station.index");
    std::filesystem::remove("route.block");
    std::filesystem::remove("route.index");
    std::filesystem::remove("pending.block");
    std::filesystem::remove("pending.index");
    std::filesystem::remove("users.block");
    std::filesystem::remove("users.index");

    result.success = true;
    result.message = "Database cleaned successfully";
    result.data = "0";

  } catch (const std::exception& e) {
    result.success = false;
    result.message = "Error cleaning database: " + std::string(e.what());
    result.data = "-1";
  }

  return result;
}

DataNumQueryHandler::DataNumQueryHandler(TrainManager& tm, UserManager& um, OrderManager& om)
    : train_manager(tm), user_manager(um), order_manager(om) {}

void DataNumQueryHandler::execute(const ParamMap& params,
                                   const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::cout << train_manager.queryTrainCount() << ' '
            << user_manager.queryUserCount() << ' '
            << order_manager.queryOrderCount() << '\n';
}

CommandResult DataNumQueryHandler::executeForWeb(const ParamMap& params,
                                                const std::string& timestamp) {
  CommandResult result;
  
  try {
    int train_count = train_manager.queryTrainCount();
    int user_count = user_manager.queryUserCount();
    int order_count = order_manager.queryOrderCount();
    
    // 构造JSON格式的返回数据
    std::string data = "{\"train_count\":" + std::to_string(train_count) + 
                      ",\"user_count\":" + std::to_string(user_count) + 
                      ",\"order_count\":" + std::to_string(order_count) + "}";
    
    result.success = true;
    result.message = "Statistics retrieved successfully";
    result.data = data;
    
  } catch (const std::exception& e) {
    result.success = false;
    result.message = "Error retrieving statistics: " + std::string(e.what());
    result.data = "{}";
  }
  
  return result;
}
