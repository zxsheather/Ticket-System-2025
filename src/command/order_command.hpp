#pragma once
#include "../controller/order_manager.hpp"
#include "../controller/seat_manager.hpp"
#include "../controller/train_manager.hpp"
#include "../controller/user_manager.hpp"
#include "../model/ticket.hpp"
#include "command_system.hpp"

enum ComparisonOrder { TIME, COST };

struct TimeComparatorForQuery {
  bool operator()(const TicketInfo& a, const TicketInfo& b) const {
    if (a.minutes != b.minutes) {
      return a.minutes < b.minutes;
    }
    return a.train_id < b.train_id;
  }
};
struct CostComparatorForQuery {
  bool operator()(const TicketInfo& a, const TicketInfo& b) const {
    if (a.price != b.price) {
      return a.price < b.price;
    }
    return a.train_id < b.train_id;
  }
};

class QueryTicketHandler : public CommandHandler {
 private:
  TrainManager& train_manager;
  SeatManager& seat_manager;

  struct TicketOrder {
    int value;
    int index;
    FixedString<20> train_id;
    bool operator<(const TicketOrder& other) const {
      return value < other.value ||
             (value == other.value && train_id < other.train_id);
    }
    bool operator>(const TicketOrder& other) const {
      return value > other.value ||
             (value == other.value && train_id > other.train_id);
    }
    bool operator==(const TicketOrder& other) const {
      return value == other.value && train_id == other.train_id;
    }
  };

 public:
  QueryTicketHandler(TrainManager& train_manager, SeatManager& seat_manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};

class BuyTicketHandler : public CommandHandler {
 private:
  TrainManager& train_manager;
  SeatManager& seat_manager;
  UserManager& user_manager;
  OrderManager& order_manager;

 public:
  BuyTicketHandler(TrainManager& train_manager, SeatManager& seat_manager,
                   UserManager& user_manager, OrderManager& order_manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};

class QueryOrderHandler : public CommandHandler {
 private:
  OrderManager& order_manager;
  UserManager& user_manager;

 public:
  QueryOrderHandler(OrderManager& order_manager, UserManager& user_manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};

class RefundTicketHandler : public CommandHandler {
 private:
  OrderManager& order_manager;
  UserManager& user_manager;
  TrainManager& train_manager;
  SeatManager& seat_manager;

 public:
  RefundTicketHandler(OrderManager& order_manager, UserManager& user_manager,
                      TrainManager& train_manager, SeatManager& seat_manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};