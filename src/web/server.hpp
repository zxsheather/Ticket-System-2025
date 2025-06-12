#ifndef WEB_SERVER_HPP
#define WEB_SERVER_HPP

#include <crow.h>
#include <map>
#include <string>
#include "../controller/user_manager.hpp"
#include "../controller/train_manager.hpp"
#include "../controller/seat_manager.hpp"
#include "../controller/order_manager.hpp"
#include "../command/command_system.hpp"

class WebServer {
private:
    crow::SimpleApp app;
    UserManager& user_manager;
    TrainManager& train_manager;
    SeatManager& seat_manager;
    OrderManager& order_manager;
    CommandSystem& command_system;
    
    // 会话管理 - sessionId -> username
    std::map<std::string, std::string> sessions;
    
public:
    WebServer(UserManager& um, TrainManager& tm, SeatManager& sm, OrderManager& om, CommandSystem& cs);
    
    void setupRoutes();
    void setupStaticFiles();
    void setupCORS();
    
    void run(int port = 8080);
    
private:
    // 工具方法
    std::string generateSessionId();
    bool isAuthenticated(const crow::request& req, std::string& username);
    bool hasAdminPrivilege(const std::string& username);
    crow::response jsonResponse(const crow::json::wvalue& data, int status = 200);
    crow::response errorResponse(const std::string& message, int status = 400);
    crow::response commandResultToResponse(const CommandResult& result);
    
    // API处理方法
    crow::response handleLogin(const crow::request& req);
    crow::response handleRegister(const crow::request& req);
    crow::response handleLogout(const crow::request& req);
    crow::response handleProfile(const crow::request& req);
    crow::response handleUpdateProfile(const crow::request& req);
    
    crow::response handleSearchTickets(const crow::request& req);
    crow::response handleSearchTransfer(const crow::request& req);
    
    crow::response handleBuyTicket(const crow::request& req);
    crow::response handleQueryOrders(const crow::request& req);
    crow::response handleRefundTicket(const crow::request& req, const std::string& order_id);
    
    // 管理员功能
    crow::response handleAddTrain(const crow::request& req);
    crow::response handleDeleteTrain(const crow::request& req, const std::string& train_id);
    crow::response handleReleaseTrain(const crow::request& req, const std::string& train_id);
    crow::response handleGetAllTrains(const crow::request& req);
    crow::response handleSearchTrains(const crow::request& req);
    crow::response handleGetSystemStats(const crow::request& req);
};

#endif
