#pragma once

#include "../controller/seat_manager.hpp"
#include "../controller/train_manager.hpp"
#include "command_system.hpp"

class AddTrainHandler : public CommandHandler {
 private:
  TrainManager& train_manager;

 public:
  AddTrainHandler(TrainManager& manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};

class DeleteTrainHandler : public CommandHandler {
 private:
  TrainManager& train_manager;

 public:
  DeleteTrainHandler(TrainManager& manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};

class ReleaseTrainHandler : public CommandHandler {
 private:
  TrainManager& train_manager;
  SeatManager& seat_manager;

 public:
  ReleaseTrainHandler(TrainManager& manager, SeatManager& seat_manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
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
  void execute(const ParamMap& params, const std::string& timestamp) override;
};

class QueryTransferHandler : public CommandHandler {
 private:
  TrainManager& train_manager;
  SeatManager& seat_manager;

 public:
  QueryTransferHandler(TrainManager& train_manager, SeatManager& seat_manager);
  void execute(const ParamMap& params, const std::string& timestamp) override;
};
