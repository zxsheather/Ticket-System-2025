#include "system_command.hpp"

#include <iostream>

void ExitHandler::execute(const ParamMap& params,
                          const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::cout << "bye";
  // exit(0);
}