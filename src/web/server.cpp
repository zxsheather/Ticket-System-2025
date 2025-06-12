#include "server.hpp"
#include <random>
#include <sstream>
#include <chrono>
#include <fstream>
#include <iostream>

WebServer::WebServer(UserManager& um, TrainManager& tm, SeatManager& sm, OrderManager& om, CommandSystem& cs)
    : user_manager(um), train_manager(tm), seat_manager(sm), order_manager(om), command_system(cs) {}

void WebServer::setupCORS() {
    
}

void WebServer::setupStaticFiles() {
    // 主页
    CROW_ROUTE(app, "/")
    ([](){
        std::ifstream file("src/web/static/index.html");
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            file.close();
            return content;
        }
        return std::string("<!DOCTYPE html><html><head><title>Train Booking System</title></head><body><h1>Train Booking System</h1><p>Frontend not found</p></body></html>");
    });
    
    // 静态文件服务
    CROW_ROUTE(app, "/static/<string>")
    ([](const std::string& path){
        std::string full_path = "src/web/static/" + path;
        std::ifstream file(full_path);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            file.close();
            return content;
        }
        return std::string("File not found");
    });
}

void WebServer::setupRoutes() {
    // 用户认证相关路由
    CROW_ROUTE(app, "/api/user/login").methods(crow::HTTPMethod::Post)
    ([this](const crow::request& req){
        return handleLogin(req);
    });
    
    CROW_ROUTE(app, "/api/user/register").methods(crow::HTTPMethod::Post)
    ([this](const crow::request& req){
        return handleRegister(req);
    });
    
    CROW_ROUTE(app, "/api/user/logout").methods(crow::HTTPMethod::Post)
    ([this](const crow::request& req){
        return handleLogout(req);
    });
    
    CROW_ROUTE(app, "/api/user/profile").methods(crow::HTTPMethod::Get)
    ([this](const crow::request& req){
        return handleProfile(req);
    });
    
    CROW_ROUTE(app, "/api/user/profile").methods(crow::HTTPMethod::Put)
    ([this](const crow::request& req){
        return handleUpdateProfile(req);
    });
    
    // 票务查询路由
    CROW_ROUTE(app, "/api/tickets/search").methods(crow::HTTPMethod::Get)
    ([this](const crow::request& req){
        return handleSearchTickets(req);
    });
    
    CROW_ROUTE(app, "/api/tickets/transfer").methods(crow::HTTPMethod::Get)
    ([this](const crow::request& req){
        return handleSearchTransfer(req);
    });
    
    // 订单相关路由
    CROW_ROUTE(app, "/api/orders").methods(crow::HTTPMethod::Get)
    ([this](const crow::request& req){
        return handleQueryOrders(req);
    });
    
    CROW_ROUTE(app, "/api/orders").methods(crow::HTTPMethod::Post)
    ([this](const crow::request& req){
        return handleBuyTicket(req);
    });
    
    CROW_ROUTE(app, "/api/orders/<string>/refund").methods(crow::HTTPMethod::Post)
    ([this](const crow::request& req, const std::string& order_id){
        return handleRefundTicket(req, order_id);
    });
    
    // 管理员路由
    CROW_ROUTE(app, "/api/admin/trains").methods(crow::HTTPMethod::Post)
    ([this](const crow::request& req){
        return handleAddTrain(req);
    });
    
    CROW_ROUTE(app, "/api/admin/trains").methods(crow::HTTPMethod::Get)
    ([this](const crow::request& req){
        return handleGetAllTrains(req);
    });
    
    CROW_ROUTE(app, "/api/admin/trains/search").methods(crow::HTTPMethod::Get)
    ([this](const crow::request& req){
        return handleSearchTrains(req);
    });
    
    CROW_ROUTE(app, "/api/admin/trains/<string>").methods(crow::HTTPMethod::Delete)
    ([this](const crow::request& req, const std::string& train_id){
        return handleDeleteTrain(req, train_id);
    });
    
    CROW_ROUTE(app, "/api/admin/trains/<string>/release").methods(crow::HTTPMethod::Post)
    ([this](const crow::request& req, const std::string& train_id){
        return handleReleaseTrain(req, train_id);
    });
    
    CROW_ROUTE(app, "/api/admin/stats").methods(crow::HTTPMethod::Get)
    ([this](const crow::request& req){
        return handleGetSystemStats(req);
    });
}

// 工具方法实现
std::string WebServer::generateSessionId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

bool WebServer::isAuthenticated(const crow::request& req, std::string& username) {
    auto session_header = req.get_header_value("X-Session-Token");
    if (session_header.empty()) return false;
    
    auto it = sessions.find(session_header);
    if (it == sessions.end()) return false;
    
    username = it->second;
    return true;
}

bool WebServer::hasAdminPrivilege(const std::string& username) {
    try {
        // 通过查询用户资料来获取权限信息
        ParamMap params;
        params.set('c', username);  // current user
        params.set('u', username);  // target user (same as current)
        
        // 生成当前时间戳
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        CommandResult result = command_system.executeForWeb("query_profile", params, timestamp_str);
        
        if (result.success) {
            // 解析result.data中的用户信息: "username name mail_addr privilege"
            std::istringstream iss(result.data);
            std::string parsed_username, name, mail_addr, privilege_str;
            iss >> parsed_username >> name >> mail_addr >> privilege_str;
            
            int privilege = std::stoi(privilege_str);
            return privilege >= 5;  // 管理员权限级别
        }
        return false;
    } catch (const std::exception& e) {
        return false;
    }
}

crow::response WebServer::jsonResponse(const crow::json::wvalue& data, int status) {
    crow::response res(status);
    res.add_header("Content-Type", "application/json");
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type, X-Session-Token");
    res.write(data.dump());
    return res;
}

crow::response WebServer::errorResponse(const std::string& message, int status) {
    crow::json::wvalue error_data;
    error_data["success"] = false;
    error_data["error"] = message;
    return jsonResponse(error_data, status);
}

crow::response WebServer::commandResultToResponse(const CommandResult& result) {
    crow::json::wvalue response_data;
    response_data["success"] = result.success;
    response_data["message"] = result.message;
    response_data["count"] = result.count;
    
    if (!result.data.empty()) {
        response_data["data"] = result.data;
    }
    
    if (!result.list_data.empty()) {
        response_data["list"] = crow::json::wvalue::list();
        for (size_t i = 0; i < result.list_data.size(); ++i) {
            response_data["list"][i] = result.list_data[i];
        }
    }
    
    return jsonResponse(response_data, result.success ? 200 : 400);
}

// Helper function to parse ticket format and convert to JSON
crow::json::wvalue parseTicketString(const std::string& ticketStr) {
    crow::json::wvalue ticket;
    
    // Debug: print the actual ticket string format
    std::cerr << "Parsing ticket string: '" << ticketStr << "'" << std::endl;
    
    // Parse the actual ticket format: 
    // "train_id from_station start_date start_time -> to_station end_date end_time price seats"
    std::istringstream iss(ticketStr);
    std::string train_id, from_station, start_date, start_time, arrow, to_station, end_date, end_time;
    int price, seats;
    
    if (iss >> train_id >> from_station >> start_date >> start_time >> arrow >> to_station >> end_date >> end_time >> price >> seats) {
        ticket["train_id"] = train_id;
        ticket["from"] = from_station;
        ticket["to"] = to_station;
        ticket["start_time"] = start_date + " " + start_time;
        ticket["end_time"] = end_date + " " + end_time;
        ticket["price"] = price;
        ticket["seats"] = seats;
        
        // Calculate travel time based on start and end times
        // This is a simplified calculation - assumes same day travel
        ticket["travel_time"] = 180; // placeholder - could parse times properly later
        
        std::cerr << "Successfully parsed ticket: " << train_id << " " << from_station << " -> " << to_station << std::endl;
    } else {
        std::cerr << "Failed to parse ticket string: '" << ticketStr << "'" << std::endl;
        // Return empty ticket object on parse failure
        ticket["train_id"] = "PARSE_ERROR";
        ticket["from"] = "ERROR";
        ticket["to"] = "ERROR";
        ticket["start_time"] = "ERROR";
        ticket["end_time"] = "ERROR";
        ticket["price"] = 0;
        ticket["seats"] = 0;
        ticket["travel_time"] = 0;
    }
    
    return ticket;
}

// Helper function to parse order format and convert to JSON  
crow::json::wvalue parseOrderString(const std::string& orderStr) {
    crow::json::wvalue order;
    
    // Debug: print the actual order string format
    std::cerr << "Parsing order string: '" << orderStr << "'" << std::endl;
    
    // Parse the order format: "[status] train_id from start_time -> to end_time price quantity"
    // Example: "[success] G1234 Beijing 06-12 08:00 -> Shanghai 06-12 14:00 350 2"
    
    std::istringstream iss(orderStr);
    std::string status_bracket, train_id, from, start_date, start_time, arrow, to, end_date, end_time;
    int price, quantity;
    
    if (iss >> status_bracket >> train_id >> from >> start_date >> start_time >> arrow >> to >> end_date >> end_time >> price >> quantity) {
        // Extract status from bracket (remove [ and ])
        std::string status = status_bracket.substr(1, status_bracket.length() - 2);
        
        order["order_id"] = "1"; // Placeholder - could be generated or retrieved
        order["train_id"] = train_id;
        order["from"] = from;
        order["to"] = to;
        order["start_time"] = start_date + " " + start_time;
        order["end_time"] = end_date + " " + end_time;
        order["price"] = price;
        order["quantity"] = quantity;
        order["status"] = status;
        
        std::cerr << "Successfully parsed order: " << train_id << " " << from << " -> " << to << " status: " << status << std::endl;
    } else {
        std::cerr << "Failed to parse order string: '" << orderStr << "'" << std::endl;
        // Return error order object on parse failure
        order["order_id"] = "ERROR";
        order["train_id"] = "PARSE_ERROR";
        order["from"] = "ERROR";
        order["to"] = "ERROR";
        order["start_time"] = "ERROR";
        order["end_time"] = "ERROR";
        order["price"] = 0;
        order["quantity"] = 0;
        order["status"] = "error";
    }
    
    return order;
}

crow::response WebServer::handleLogin(const crow::request& req) {
    try {
        auto body = crow::json::load(req.body);
        if (!body) {
            return errorResponse("Invalid JSON", 400);
        }
        
        std::string username = body["username"].s();
        std::string password = body["password"].s();
        
        ParamMap params;
        params.set('u', username);
        params.set('p', password);
        
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        CommandResult result = command_system.executeForWeb("login", params, timestamp_str);
        
        if (result.success) {
            int privilege = std::stoi(result.data);
            
            std::string session_id = generateSessionId();
            sessions[session_id] = username;
            
            crow::json::wvalue response_data;
            response_data["success"] = true;
            response_data["session_token"] = session_id;
            response_data["privilege"] = privilege;
            response_data["username"] = username;
            response_data["message"] = result.message;
            
            return jsonResponse(response_data);
        } else {
            return errorResponse(result.message, 401);
        }
    } catch (const std::exception& e) {
        return errorResponse("Login failed", 500);
    }
}

crow::response WebServer::handleRegister(const crow::request& req) {
    try {
        auto body = crow::json::load(req.body);
        if (!body) {
            return errorResponse("Invalid JSON", 400);
        }
        
        std::string cur_username = body.has("cur_username") ? std::string(body["cur_username"].s()) : std::string("");
        std::string username = body["username"].s();
        std::string password = body["password"].s();
        std::string name = body["name"].s();
        std::string mail_addr = body["mail_addr"].s();
        int privilege = body.has("privilege") ? body["privilege"].i() : 1;
        
        // 创建参数映射
        ParamMap params;
        if (!cur_username.empty()) {
            params.set('c', cur_username);
        }
        params.set('u', username);
        params.set('p', password);
        params.set('n', name);
        params.set('m', mail_addr);
        if (privilege != 1) {
            params.set('g', std::to_string(privilege));
        }
        
        // 生成当前时间戳
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        // 使用 Command Handler
        CommandResult result = command_system.executeForWeb("add_user", params, timestamp_str);
        
        if (result.success) {
            crow::json::wvalue response_data;
            response_data["success"] = true;
            response_data["message"] = result.message;
            return jsonResponse(response_data);
        } else {
            return errorResponse(result.message, 400);
        }
    } catch (const std::exception& e) {
        return errorResponse("Registration failed", 500);
    }
}

crow::response WebServer::handleLogout(const crow::request& req) {
    auto session_header = req.get_header_value("X-Session-Token");
    if (session_header.empty()) {
        return errorResponse("No session token provided", 401);
    }
    
    auto it = sessions.find(session_header);
    if (it == sessions.end()) {
        return errorResponse("Invalid session token", 401);
    }
    
    std::string username = it->second;
    
    // 创建参数映射
    ParamMap params;
    params.set('u', username);
    
    // 生成当前时间戳
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    std::string timestamp_str = std::to_string(timestamp);
    
    // 使用 Command Handler
    CommandResult result = command_system.executeForWeb("logout", params, timestamp_str);
    
    // 无论CommandSystem返回什么，都要清除会话
    sessions.erase(it);
    
    if (result.success) {
        crow::json::wvalue response_data;
        response_data["success"] = true;
        response_data["message"] = result.message;
        return jsonResponse(response_data);
    } else {
        // 即使logout命令失败，会话已经被清除，所以仍然返回成功
        crow::json::wvalue response_data;
        response_data["success"] = true;
        response_data["message"] = "Logged out successfully (session cleared)";
        return jsonResponse(response_data);
    }
}

crow::response WebServer::handleProfile(const crow::request& req) {
    std::string current_user;
    if (!isAuthenticated(req, current_user)) {
        return errorResponse("Authentication required", 401);
    }
    
    std::string target_user = req.url_params.get("username") ? 
                              req.url_params.get("username") : current_user;
    
    // 创建参数映射
    ParamMap params;
    params.set('c', current_user);
    params.set('u', target_user);
    
    // 生成当前时间戳
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    std::string timestamp_str = std::to_string(timestamp);
    
    // 使用 Command Handler
    CommandResult result = command_system.executeForWeb("query_profile", params, timestamp_str);
    
    if (result.success) {
        // 解析result.data中的用户信息: "username name mail_addr privilege"
        std::istringstream iss(result.data);
        std::string username, name, mail_addr, privilege_str;
        iss >> username >> name >> mail_addr >> privilege_str;
        
        crow::json::wvalue response_data;
        response_data["success"] = true;
        response_data["message"] = result.message;
        response_data["profile"]["username"] = username;
        response_data["profile"]["name"] = name;
        response_data["profile"]["mail_addr"] = mail_addr;
        response_data["profile"]["privilege"] = std::stoi(privilege_str);
        
        return jsonResponse(response_data);
    } else {
        return errorResponse(result.message, 400);
    }
}

// 更新用户资料 - 使用 Command Handler
crow::response WebServer::handleUpdateProfile(const crow::request& req) {
    std::string current_user;
    if (!isAuthenticated(req, current_user)) {
        return errorResponse("Authentication required", 401);
    }
    
    try {
        auto body = crow::json::load(req.body);
        if (!body) {
            return errorResponse("Invalid JSON", 400);
        }
        
        std::string target_user = body.has("username") ? std::string(body["username"].s()) : current_user;
        std::string password = body.has("password") ? std::string(body["password"].s()) : std::string("");
        std::string name = body.has("name") ? std::string(body["name"].s()) : std::string("");
        std::string mail_addr = body.has("mail_addr") ? std::string(body["mail_addr"].s()) : std::string("");
        int privilege = body.has("privilege") ? body["privilege"].i() : -1;
        
        // 创建参数映射
        ParamMap params;
        params.set('c', current_user);
        params.set('u', target_user);
        if (!password.empty()) {
            params.set('p', password);
        }
        if (!name.empty()) {
            params.set('n', name);
        }
        if (!mail_addr.empty()) {
            params.set('m', mail_addr);
        }
        if (privilege != -1) {
            params.set('g', std::to_string(privilege));
        }
        
        // 生成当前时间戳
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        // 使用 Command Handler
        CommandResult result = command_system.executeForWeb("modify_profile", params, timestamp_str);
        
        if (result.success) {
            crow::json::wvalue response_data;
            response_data["success"] = true;
            response_data["message"] = result.message;
            return jsonResponse(response_data);
        } else {
            return errorResponse(result.message, 400);
        }
    } catch (const std::exception& e) {
        return errorResponse("Update failed", 500);
    }
}

// 票务查询处理 - 使用 Command Handler
crow::response WebServer::handleSearchTickets(const crow::request& req) {
    try {
        std::string from = req.url_params.get("from") ? req.url_params.get("from") : "";
        std::string to = req.url_params.get("to") ? req.url_params.get("to") : "";
        std::string date_str = req.url_params.get("date") ? req.url_params.get("date") : "";
        std::string sort = req.url_params.get("sort") ? req.url_params.get("sort") : "time";
        
        if (from.empty() || to.empty() || date_str.empty()) {
            return errorResponse("Missing required parameters: from, to, date", 400);
        }
        
        // 创建参数映射
        ParamMap params;
        params.set('s', from);   // 's' for start station
        params.set('t', to);     // 't' for terminal station  
        params.set('d', date_str); // 'd' for date
        params.set('p', sort);   // 'p' for sort preference
        
        // 生成当前时间戳
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        // 使用 Command Handler
        CommandResult result = command_system.executeForWeb("query_ticket", params, timestamp_str);
        
        // Special handling for ticket search results
        if (result.success && !result.list_data.empty()) {
            crow::json::wvalue response_data;
            response_data["success"] = true;
            response_data["message"] = result.message;
            response_data["count"] = result.count;
            
            // Parse ticket strings and convert to JSON objects
            response_data["tickets"] = crow::json::wvalue::list();
            for (size_t i = 0; i < result.list_data.size(); ++i) {
                response_data["tickets"][i] = parseTicketString(result.list_data[i]);
            }
            
            return jsonResponse(response_data, 200);
        } else {
            // Return empty tickets array if no results
            crow::json::wvalue response_data;
            response_data["success"] = true;
            response_data["message"] = result.message.empty() ? "No tickets found" : result.message;
            response_data["count"] = 0;
            response_data["tickets"] = crow::json::wvalue::list();
            
            return jsonResponse(response_data, 200);
        }
        
    } catch (const std::exception& e) {
        return errorResponse("Search failed", 500);
    }
}

// 换乘查询处理 - 使用 Command Handler
crow::response WebServer::handleSearchTransfer(const crow::request& req) {
    try {
        std::string from = req.url_params.get("from") ? req.url_params.get("from") : "";
        std::string to = req.url_params.get("to") ? req.url_params.get("to") : "";
        std::string date_str = req.url_params.get("date") ? req.url_params.get("date") : "";
        std::string sort = req.url_params.get("sort") ? req.url_params.get("sort") : "time";
        
        if (from.empty() || to.empty() || date_str.empty()) {
            return errorResponse("Missing required parameters: from, to, date", 400);
        }
        
        // 创建参数映射
        ParamMap params;
        params.set('s', from);   // 's' for start station
        params.set('t', to);     // 't' for terminal station
        params.set('d', date_str); // 'd' for date
        params.set('p', sort);   // 'p' for sort preference
        
        // 生成当前时间戳
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        // 使用 Command Handler
        CommandResult result = command_system.executeForWeb("query_transfer", params, timestamp_str);
        return commandResultToResponse(result);
        
    } catch (const std::exception& e) {
        return errorResponse("Transfer search failed", 500);
    }
}

// 购票处理 - 使用 Command Handler
crow::response WebServer::handleBuyTicket(const crow::request& req) {
    std::string username;
    if (!isAuthenticated(req, username)) {
        return errorResponse("Authentication required", 401);
    }
    
    try {
        auto body = crow::json::load(req.body);
        if (!body) {
            return errorResponse("Invalid JSON", 400);
        }
        
        std::string train_id = body["train_id"].s();
        std::string date_str = body["date"].s();
        std::string from_station = body["from"].s();
        std::string to_station = body["to"].s();
        int ticket_num = body["quantity"].i();  // Frontend sends "quantity"
        bool wait_queue = body.has("queue") ? body["queue"].b() : false;  // Frontend sends "queue"
        
        // 创建参数映射
        ParamMap params;
        params.set('u', username);     // 'u' for username
        params.set('i', train_id);     // 'i' for train id
        params.set('d', date_str);     // 'd' for date
        params.set('f', from_station); // 'f' for from station
        params.set('t', to_station);   // 't' for to station
        params.set('n', std::to_string(ticket_num)); // 'n' for number of tickets
        if (wait_queue) {
            params.set('q', "true");   // 'q' for queue
        }
        
        // 生成当前时间戳
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        // 使用 Command Handler
        CommandResult result = command_system.executeForWeb("buy_ticket", params, timestamp_str);
        return commandResultToResponse(result);
        
    } catch (const std::exception& e) {
        return errorResponse("Purchase failed " + std::string(e.what()), 500);
    }
}

// 查询订单处理 - 使用 Command Handler
crow::response WebServer::handleQueryOrders(const crow::request& req) {
    std::string username;
    if (!isAuthenticated(req, username)) {
        return errorResponse("Authentication required", 401);
    }
    
    try {
        ParamMap params;
        params.set('u', username); 
        
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        CommandResult result = command_system.executeForWeb("query_order", params, timestamp_str);
        
        if (result.success) {
            crow::json::wvalue response_data;
            response_data["success"] = true;
            response_data["message"] = result.message;
            response_data["count"] = result.count;
            
            // 解析订单字符串并转换为 JSON 对象
            response_data["orders"] = crow::json::wvalue::list();
            for (size_t i = 0; i < result.list_data.size(); ++i) {
                response_data["orders"][i] = parseOrderString(result.list_data[i]);
            }
            
            return jsonResponse(response_data, 200);
        } else {
            return errorResponse(result.message, 400);
        }
        
    } catch (const std::exception& e) {
        return errorResponse("Query orders failed", 500);
    }
}

// 退票处理 - 使用 Command Handler
crow::response WebServer::handleRefundTicket(const crow::request& req, const std::string& order_id) {
    std::string username;
    if (!isAuthenticated(req, username)) {
        return errorResponse("Authentication required", 401);
    }
    
    try {
        // 创建参数映射
        ParamMap params;
        params.set('u', username); // 'u' for username
        params.set('n', order_id);  // 'n' for order number
        
        // 生成当前时间戳
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        // 使用 Command Handler
        CommandResult result = command_system.executeForWeb("refund_ticket", params, timestamp_str);
        return commandResultToResponse(result);
        
    } catch (const std::exception& e) {
        return errorResponse("Refund failed", 500);
    }
}

// 管理员功能 - 添加车次 - 使用 Command Handler
crow::response WebServer::handleAddTrain(const crow::request& req) {
    std::string username;
    if (!isAuthenticated(req, username)) {
        return errorResponse("Authentication required", 401);
    }
    
    // 检查管理员权限
    if (!hasAdminPrivilege(username)) {
        return errorResponse("Admin privilege required", 403);
    }
    
    try {
        auto body = crow::json::load(req.body);
        if (!body) {
            return errorResponse("Invalid JSON", 400);
        }
        
        // 解析请求数据 - 直接使用前端发送的管道分隔字符串
        std::string train_id = body["train_id"].s();
        std::string stations_str = body["stations"].s();
        std::string prices_str = body["prices"].s();
        std::string travel_times_str = body["travel_times"].s();
        std::string stopover_times_str = body["stopover_times"].s();
        std::string release_date = body["release_date"].s();
        int seat_num = body["seat_num"].i();
        std::string sale_date = body["sale_date"].s();
        std::string start_time = body.has("start_time") ? body["start_time"].s() : std::string("08:00");
        
        char type = 'G';
        if (body.has("type")) {
            std::string type_str = body["type"].s();
            if (!type_str.empty()) {
                type = type_str[0];
            }
        }
        
        // 验证数据完整性
        if (train_id.empty() || stations_str.empty() || prices_str.empty() || travel_times_str.empty() || sale_date.empty()) {
            return errorResponse("Missing required train data", 400);
        }
        
        // 计算站点数量（用于验证）
        int station_count = std::count(stations_str.begin(), stations_str.end(), '|') + 1;
        
        // 创建参数映射 - 按照添加车次命令的格式
        ParamMap params;
        params.set('i', train_id);
        params.set('n', std::to_string(station_count));
        params.set('m', std::to_string(seat_num));
        params.set('s', stations_str);
        params.set('p', prices_str);
        params.set('x', start_time);
        params.set('t', travel_times_str);
        params.set('o', stopover_times_str);
        params.set('d', sale_date);
        params.set('y', std::string(1, type));
        
        // 生成当前时间戳
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        // 使用 Command Handler
        CommandResult result = command_system.executeForWeb("add_train", params, timestamp_str);
        
        if (result.success) {
            crow::json::wvalue response_data;
            response_data["success"] = true;
            response_data["message"] = result.message;
            response_data["train_id"] = train_id;
            
            return jsonResponse(response_data);
        } else {
            return errorResponse(result.message, 400);
        }
        
    } catch (const std::exception& e) {
        return errorResponse("Add train failed: Invalid request format", 500);
    }
}

// 管理员功能 - 删除车次 - 使用 Command Handler
crow::response WebServer::handleDeleteTrain(const crow::request& req, const std::string& train_id) {
    std::string username;
    if (!isAuthenticated(req, username)) {
        return errorResponse("Authentication required", 401);
    }
    
    // 检查管理员权限
    if (!hasAdminPrivilege(username)) {
        return errorResponse("Admin privilege required", 403);
    }
    
    try {
        if (train_id.empty()) {
            return errorResponse("Train ID is required", 400);
        }
        
        // 创建参数映射
        ParamMap params;
        params.set('i', train_id);
        
        // 生成当前时间戳
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        // 使用 Command Handler
        CommandResult result = command_system.executeForWeb("delete_train", params, timestamp_str);
        
        if (result.success) {
            crow::json::wvalue response_data;
            response_data["success"] = true;
            response_data["message"] = result.message;
            response_data["train_id"] = train_id;
            
            return jsonResponse(response_data);
        } else {
            return errorResponse(result.message, 400);
        }
        
    } catch (const std::exception& e) {
        return errorResponse("Delete train failed", 500);
    }
}

crow::response WebServer::handleReleaseTrain(const crow::request& req, const std::string& train_id) {
    std::string username;
    if (!isAuthenticated(req, username)) {
        return errorResponse("Authentication required", 401);
    }
    
    // 检查管理员权限
    if (!hasAdminPrivilege(username)) {
        return errorResponse("Admin privilege required", 403);
    }
    
    try {
        if (train_id.empty()) {
            return errorResponse("Train ID is required", 400);
        }
        
        // 创建参数映射
        ParamMap params;
        params.set('i', train_id);
        
        // 生成当前时间戳
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::string timestamp_str = std::to_string(timestamp);
        
        // 使用 Command Handler
        CommandResult result = command_system.executeForWeb("release_train", params, timestamp_str);
        
        if (result.success) {
            crow::json::wvalue response_data;
            response_data["success"] = true;
            response_data["message"] = result.message;
            response_data["train_id"] = train_id;
            
            return jsonResponse(response_data);
        } else {
            return errorResponse(result.message, 400);
        }
        
    } catch (const std::exception& e) {
        return errorResponse("Release train failed", 500);
    }
}

crow::response WebServer::handleGetAllTrains(const crow::request& req) {
    std::string username;
    if (!isAuthenticated(req, username)) {
        return errorResponse("Authentication required", 401);
    }
    
    // 检查管理员权限
    if (!hasAdminPrivilege(username)) {
        return errorResponse("Admin privilege required", 403);
    }
    
    try {
        // 由于没有直接获取所有车次的命令，这里返回占位符实现
        // 在实际应用中，可以通过遍历火车管理器或添加新的命令来实现
        crow::json::wvalue response_data;
        response_data["success"] = true;
        response_data["trains"] = crow::json::wvalue::list();
        response_data["message"] = "Train list loaded successfully";
        
        // 这里可以添加实际的车次获取逻辑
        // 例如：通过 train_manager 获取所有车次
        
        return jsonResponse(response_data);
        
    } catch (const std::exception& e) {
        return errorResponse("Failed to get trains", 500);
    }
}

// 管理员功能 - 搜索车次
crow::response WebServer::handleSearchTrains(const crow::request& req) {
    std::string username;
    if (!isAuthenticated(req, username)) {
        return errorResponse("Authentication required", 401);
    }
    
    // 检查管理员权限
    if (!hasAdminPrivilege(username)) {
        return errorResponse("Admin privilege required", 403);
    }
    
    try {
        std::string query = req.url_params.get("q") ? req.url_params.get("q") : "";
        
        if (query.empty()) {
            return errorResponse("Search query is required", 400);
        }
        
        // 目前返回空结果，实际应用中可以通过train_manager搜索
        crow::json::wvalue response_data;
        response_data["success"] = true;
        response_data["trains"] = crow::json::wvalue::list();
        response_data["message"] = "Search completed";
        
        return jsonResponse(response_data);
        
    } catch (const std::exception& e) {
        return errorResponse("Search failed", 500);
    }
}

// 管理员功能 - 获取系统统计 - 使用 Command Handler
crow::response WebServer::handleGetSystemStats(const crow::request& req) {
    std::string username;
    if (!isAuthenticated(req, username)) {
        return errorResponse("Authentication required", 401);
    }
    
    // 检查管理员权限
    if (!hasAdminPrivilege(username)) {
        return errorResponse("Admin privilege required", 403);
    }
    
    try {
        // 由于没有专门的系统统计命令，这里返回占位符实现
        // 在实际应用中，需要实现专门的统计命令或通过多个查询组合统计信息
        crow::json::wvalue response_data;
        response_data["success"] = true;
        response_data["stats"]["total_users"] = 0; // 需要通过用户查询获取
        response_data["stats"]["total_trains"] = 0; // 需要通过车次查询获取
        response_data["stats"]["released_trains"] = 0;
        response_data["stats"]["total_orders"] = 0; // 需要通过订单查询获取
        response_data["stats"]["pending_orders"] = 0;
        response_data["stats"]["total_revenue"] = 0;
        response_data["message"] = "System statistics feature not fully implemented yet";
        
        return jsonResponse(response_data);
        
    } catch (const std::exception& e) {
        return errorResponse("Failed to get stats", 500);
    }
}

void WebServer::run(int port) {
    setupCORS();
    setupStaticFiles();
    setupRoutes();
    
    std::cout << "Starting web server on port " << port << std::endl;
    app.port(port).multithreaded().run();
}