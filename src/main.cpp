// The entrance for the program

#include <iostream>

#include "command/command_system.hpp"
#include "command/system_command.hpp"
#include "command/user_command.hpp"

int main() {
  CommandSystem command_system;
  UserManager user_manager;
  command_system.registerHandler("login", new LoginHandler(user_manager));
  command_system.registerHandler("add_user", new AddUserHandler(user_manager));
  command_system.registerHandler("logout", new LogoutHandler(user_manager));
  command_system.registerHandler("query_profile",
                                 new QueryProfileHandler(user_manager));
  command_system.registerHandler("modify_profile",
                                 new ModifyProfileHandler(user_manager));
  command_system.registerHandler("exit", new ExitHandler());

  std::string line;
  while (getline(std::cin, line)) {
    std::string timestamp;
    std::string result = command_system.parseAndExecute(line, timestamp);
    std::cout << '[' + timestamp + "] " + result << '\n';
    if (result == "bye") {
      break;
    }
  }
  return 0;
}