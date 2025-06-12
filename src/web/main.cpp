#include "server.hpp"
#include "../controller/user_manager.hpp"
#include "../controller/train_manager.hpp"
#include "../controller/seat_manager.hpp"
#include "../controller/order_manager.hpp"
#include "../command/command_system.hpp"
#include "../command/user_command.hpp"
#include "../command/train_command.hpp"
#include "../command/order_command.hpp"
#include "../command/system_command.hpp"
#include <iostream>

int main() {
    try {
        UserManager user_manager;
        TrainManager train_manager;
        SeatManager seat_manager;
        OrderManager order_manager;
        
        CommandSystem command_system;
        
        command_system.registerHandler("login", new LoginHandler(user_manager));
        command_system.registerHandler("add_user", new AddUserHandler(user_manager));
        command_system.registerHandler("logout", new LogoutHandler(user_manager));
        command_system.registerHandler("query_profile", new QueryProfileHandler(user_manager));
        command_system.registerHandler("modify_profile", new ModifyProfileHandler(user_manager));
        
        command_system.registerHandler("add_train", new AddTrainHandler(train_manager));
        command_system.registerHandler("delete_train", new DeleteTrainHandler(train_manager));
        command_system.registerHandler("release_train", new ReleaseTrainHandler(train_manager, seat_manager));
        command_system.registerHandler("query_train", new QueryTrainHandler(train_manager, seat_manager));
        command_system.registerHandler("query_transfer", new QueryTransferHandler(train_manager, seat_manager));
        
        command_system.registerHandler("query_ticket", new QueryTicketHandler(train_manager, seat_manager));
        command_system.registerHandler("buy_ticket", new BuyTicketHandler(train_manager, seat_manager, user_manager, order_manager));
        command_system.registerHandler("query_order", new QueryOrderHandler(order_manager, user_manager));
        command_system.registerHandler("refund_ticket", new RefundTicketHandler(order_manager, user_manager, train_manager, seat_manager));
        
        command_system.registerHandler("clean", new CleanHandler());
        command_system.registerHandler("exit", new ExitHandler());
        command_system.registerHandler("data_num_query", new DataNumQueryHandler(train_manager, user_manager, order_manager));
        
        WebServer server(user_manager, train_manager, seat_manager, order_manager, command_system);
        
        std::cout << "Starting web server on port 8080..." << std::endl;
        std::cout << "Open http://localhost:8080 in your browser" << std::endl;
        
        server.run(8080);
        
    } catch (const std::exception& e) {
        std::cerr << "Error starting web server: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
