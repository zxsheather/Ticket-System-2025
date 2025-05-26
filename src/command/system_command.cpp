
#include "system_command.hpp"

std::string ExitHandler::execute(const ParamMap& params,
                                 const std::string& timestamp) {
  return "bye";
}