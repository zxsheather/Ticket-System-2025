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
  std::filesystem::remove("user.block");
  std::filesystem::remove("user.index");
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
    std::filesystem::remove("user.block");
    std::filesystem::remove("user.index");

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
