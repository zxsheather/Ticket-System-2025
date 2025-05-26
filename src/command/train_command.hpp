#pragma once

#include "../controller/order_manager.hpp"
#include "../controller/seat_manager.hpp"
#include "../controller/train_manager.hpp"
#include "../controller/user_manager.hpp"
#include "../model/ticket.hpp"
#include "system_command.hpp"

struct TimeComparatorForQuery {
  bool operator()(const TicketInfo& a, const TicketInfo& b) const {
    if (a.minutes != b.minutes) {
      return a.minutes > b.minutes;
    }
    return a.train_id > b.train_id;
  }
};
struct CostComparatorForQuery {
  bool operator()(const TicketInfo& a, const TicketInfo& b) const {
    if (a.price != b.price) {
      return a.price > b.price;
    }
    return a.train_id > b.train_id;
  }
};
struct TimeComparatorForBuy {
  bool operator()(const TicketInfo& a, const TicketInfo& b) const {
    if (a.minutes != b.minutes) {
      return a.minutes > b.minutes;
    }
    if (a.price != b.price) {
      return a.price > b.price;
    }
    return a.train_id > b.train_id;
  }
};
struct CostComparatorForBuy {
  bool operator()(const TicketInfo& a, const TicketInfo& b) const {
    if (a.price != b.price) {
      return a.price > b.price;
    }
    if (a.minutes != b.minutes) {
      return a.minutes > b.minutes;
    }
    return a.train_id > b.train_id;
  }
};

class AddTrainHandler : public CommandHandler {
 private:
  TrainManager& train_manager;

 public:
  AddTrainHandler(TrainManager& manager);
  std::string execute(const ParamMap& params,
                      const std::string& timestamp) override;
};

class DeleteTrainHandler : public CommandHandler {
 private:
  TrainManager& train_manager;

 public:
  DeleteTrainHandler(TrainManager& manager);
  std::string execute(const ParamMap& params,
                      const std::string& timestamp) override;
};

class ReleaseTrainHandler : public CommandHandler {
 private:
  TrainManager& train_manager;
  SeatManager& seat_manager;

 public:
  ReleaseTrainHandler(TrainManager& manager, SeatManager& seat_manager);
  std::string execute(const ParamMap& params,
                      const std::string& timestamp) override;
};

class QueryTrainHandler : public CommandHandler {
 private:
  TrainManager& train_manager;
  SeatManager& seat_manager;

  std::string format(const Train& train, int* seats, const Date& date) {
    std::string result = train.train_id.toString() + ' ' + train.type + '\n';
    result += train.stations[0].toString() + " xx-xx xx:xx -> " +
              TimePoint(date, train.departure_times[0]).toString() + " 0 " +
              std::to_string(seats[0]) + '\n';
    for (int i = 1; i < train.station_num - 1; ++i) {
      result += train.stations[i].toString() + ' ' +
                TimePoint{date, train.arrival_times[i]}.toString() + " -> " +
                TimePoint(date, train.departure_times[i]).toString() + " " +
                std::to_string(train.prices[i]) + " " +
                std::to_string(seats[i]) + '\n';
    }
    result +=
        train.stations[train.station_num - 1].toString() + ' ' +
        TimePoint(date, train.arrival_times[train.station_num - 1]).toString() +
        " -> xx-xx xx:xx " +
        std::to_string(train.prices[train.station_num - 1]) + " x";
    return result;
  }

 public:
  QueryTrainHandler(TrainManager& train_manager, SeatManager& seat_manager);
  std::string execute(const ParamMap& params,
                      const std::string& timestamp) override;
};

class QueryTicketHandler : public CommandHandler {
 private:
  TrainManager& train_manager;
  SeatManager& seat_manager;

 public:
  QueryTicketHandler(TrainManager& train_manager, SeatManager& seat_manager);
  std::string execute(const ParamMap& params,
                      const std::string& timestamp) override;
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
  std::string execute(const ParamMap& params,
                      const std::string& timestamp) override;
};
