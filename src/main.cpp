// acmoj: 1867
// The entrance for the program

#include <iostream>

#include "command/command_system.hpp"
#include "command/order_command.hpp"
#include "command/system_command.hpp"
#include "command/train_command.hpp"
#include "command/user_command.hpp"

int main() {
  CommandSystem command_system;
  UserManager user_manager;
  TrainManager train_manager;
  SeatManager seat_manager;
  OrderManager order_manager;
  command_system.registerHandler("login", new LoginHandler(user_manager));
  command_system.registerHandler("add_user", new AddUserHandler(user_manager));
  command_system.registerHandler("logout", new LogoutHandler(user_manager));
  command_system.registerHandler("query_profile",
                                 new QueryProfileHandler(user_manager));
  command_system.registerHandler("modify_profile",
                                 new ModifyProfileHandler(user_manager));
  command_system.registerHandler("exit", new ExitHandler());
  command_system.registerHandler("add_train",
                                 new AddTrainHandler(train_manager));
  command_system.registerHandler("delete_train",
                                 new DeleteTrainHandler(train_manager));
  command_system.registerHandler(
      "release_train", new ReleaseTrainHandler(train_manager, seat_manager));
  command_system.registerHandler(
      "query_train", new QueryTrainHandler(train_manager, seat_manager));
  command_system.registerHandler(
      "query_ticket", new QueryTicketHandler(train_manager, seat_manager));
  command_system.registerHandler(
      "buy_ticket", new BuyTicketHandler(train_manager, seat_manager,
                                         user_manager, order_manager));
  command_system.registerHandler(
      "query_order", new QueryOrderHandler(order_manager, user_manager));

  command_system.registerHandler(
      "refund_ticket", new RefundTicketHandler(order_manager, user_manager,
                                               train_manager, seat_manager));

  command_system.registerHandler(
      "query_transfer", new QueryTransferHandler(train_manager, seat_manager));
  std::string line;

  while (getline(std::cin, line)) {
    std::string timestamp;
    std::string cmd_name;
    command_system.parseAndExecute(line, timestamp, cmd_name);
    if (cmd_name == "exit") {
      break;
    }
  }
  return 0;
}