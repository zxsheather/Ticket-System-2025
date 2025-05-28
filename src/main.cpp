// acmoj: 1867
// The entrance for the program

#include <chrono>
#include <iostream>

#include "command/command_system.hpp"
#include "command/order_command.hpp"
#include "command/system_command.hpp"
#include "command/train_command.hpp"
#include "command/user_command.hpp"

int main() {
  // freopen("test/seat_basic.in", "r", stdin);
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

  // for debugging purposes
  long long query_profile_time = 0;
  long long query_ticket_time = 0;
  long long buy_ticket_time = 0;
  long long query_order_time = 0;
  long long refund_ticket_time = 0;
  long long query_transfer_time = 0;
  long long query_train_time = 0;

  while (getline(std::cin, line)) {
    std::string timestamp;
    std::string cmd_name;
    auto start_time = std::chrono::high_resolution_clock::now();
    command_system.parseAndExecute(line, timestamp, cmd_name);
    auto end_time = std::chrono::high_resolution_clock::now();
    long long elapsed_time =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end_time -
                                                             start_time)
            .count();
    if (cmd_name == "query_ticket") {
      query_ticket_time += elapsed_time;
    } else if (cmd_name == "buy_ticket") {
      buy_ticket_time += elapsed_time;
    } else if (cmd_name == "query_order") {
      query_order_time += elapsed_time;
    } else if (cmd_name == "refund_ticket") {
      refund_ticket_time += elapsed_time;
    } else if (cmd_name == "query_transfer") {
      query_transfer_time += elapsed_time;
    } else if (cmd_name == "query_train") {
      query_train_time += elapsed_time;
    } else if (cmd_name == "query_profile") {
      query_profile_time += elapsed_time;
    } else if (cmd_name == "exit") {
      std::cerr << "Query Profile Time: " << query_profile_time / 1000000.0
                << " ms\n";
      std::cerr << "Query Ticket Time: " << query_ticket_time / 1000000.0
                << " ms\n";
      std::cerr << "Buy Ticket Time: " << buy_ticket_time / 1000000.0
                << " ms\n";
      std::cerr << "Query Order Time: " << query_order_time / 1000000.0
                << " ms\n";
      std::cerr << "Refund Ticket Time: " << refund_ticket_time / 1000000.0
                << " ms\n";
      std::cerr << "Query Transfer Time: " << query_transfer_time / 1000000.0
                << " ms\n";
      std::cerr << "Query Train Time: " << query_train_time / 1000000.0
                << " ms\n";
      break;
    }
  }
  return 0;
}