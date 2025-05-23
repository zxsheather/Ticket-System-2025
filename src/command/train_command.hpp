#pragma once

#include "../controller/seat_manager.hpp"
#include "../controller/train_manager.hpp"
#include "../controller/user_manager.hpp"
#include "system_command.hpp"

class AddTrainHandler : public CommandHandler {
 private:
  TrainManager& train_manager;

 public:
  AddTrainHandler(TrainManager& manager);
  std::string execute(const ParamMap& params) override;
};

class DeleteTrainHandler : public CommandHandler {
 private:
  TrainManager& train_manager;

 public:
  DeleteTrainHandler(TrainManager& manager);
  std::string execute(const ParamMap& params) override;
};

class ReleaseTrainHandler : public CommandHandler {
 private:
  TrainManager& train_manager;

 public:
  ReleaseTrainHandler(TrainManager& manager);
  std::string execute(const ParamMap& params) override;
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
                std::to_string(train.prices[i - 1]) + " " +
                std::to_string(seats[i]) + '\n';
    }
    result +=
        train.stations[train.station_num - 1].toString() + ' ' +
        TimePoint(date, train.arrival_times[train.station_num - 1]).toString() +
        " -> xx-xx xx:xx " +
        std::to_string(train.prices[train.station_num - 2]) + " x\n";
    return result;
  }

 public:
  QueryTrainHandler(TrainManager& train_manager, SeatManager& seat_manager);
  std::string execute(const ParamMap& params) override;
};

class BuyTicketHandler : public CommandHandler {
 private:
  TrainManager& train_manager;
  SeatManager& seat_manager;
  UserManager& user_manager;

 public:
  BuyTicketHandler(TrainManager& train_manager, SeatManager& seat_manager,
                   UserManager& user_manager);
  std::string execute(const ParamMap& params) override;
};
