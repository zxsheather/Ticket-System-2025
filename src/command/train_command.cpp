#include "train_command.hpp"

#include "../controller/seat_manager.hpp"
#include "../model/time.hpp"
#include "../utilities/parse_by_char.hpp"
#include "command_system.hpp"

AddTrainHandler::AddTrainHandler(TrainManager& manager)
    : train_manager(manager) {}
std::string AddTrainHandler::execute(const ParamMap& params,
                                     const std::string& timestamp) {
  Train train;
  train.train_id = params.get('i');
  train.station_num = std::stoi(params.get('n'));
  train.seat_num = std::stoi(params.get('m'));
  std::string stations_str = params.get('s');
  parse_by_char(stations_str, '|', train.stations);
  std::string prices_str = params.get('p');
  parse_by_char_accumulate(prices_str, '|', train.prices);
  std::string start_time_string = params.get('x');
  train.departure_times[0].hour = std::stoi(start_time_string.substr(0, 2));
  train.departure_times[0].minute = std::stoi(start_time_string.substr(3));
  train.arrival_times[0] = Time{0, 0};
  std::string travel_times_str = params.get('t');
  int travel_times[MAX_STATION_NUM - 1];
  parse_by_char(travel_times_str, '|', travel_times);

  std::string stop_over_time_str = params.get('o');
  int stop_over_times[MAX_STATION_NUM - 2] = {0};
  if (stop_over_time_str != "_") {
    parse_by_char(stop_over_time_str, '|', stop_over_times);
  }
  for (int i = 1; i < train.station_num; i++) {
    train.arrival_times[i] = train.departure_times[i - 1] + travel_times[i - 1];

    if (i < train.station_num - 1) {
      train.departure_times[i] =
          train.arrival_times[i] + stop_over_times[i - 1];
    } else {
      train.departure_times[i] = Time{0, 0};
    }
  }
  std::string sale_date_str = params.get('d');
  train.sale_date_start.month = std::stoi(sale_date_str.substr(0, 2));
  train.sale_date_start.day = std::stoi(sale_date_str.substr(3, 2));
  train.sale_date_end.month = std::stoi(sale_date_str.substr(6, 2));
  train.sale_date_end.day = std::stoi(sale_date_str.substr(9, 2));
  train.type = params.get('y')[0];
  int result = train_manager.addTrain(train);
  return std::to_string(result);
};

DeleteTrainHandler::DeleteTrainHandler(TrainManager& manager)
    : train_manager(manager) {}
std::string DeleteTrainHandler::execute(const ParamMap& params,
                                        const std::string& timestamp) {
  std::string train_id = params.get('i');
  int result = train_manager.deleteTrain(train_id);
  return std::to_string(result);
};

ReleaseTrainHandler::ReleaseTrainHandler(TrainManager& manager,
                                         SeatManager& seat_manager)
    : train_manager(manager), seat_manager(seat_manager) {}
std::string ReleaseTrainHandler::execute(const ParamMap& params,
                                         const std::string& timestamp) {
  std::string train_id = params.get('i');
  Train train;
  int result = train_manager.releaseTrain(train_id, train);
  if (result == -1) {
    return "-1";
  }
  seat_manager.initSeat(train);
  return std::to_string(result);
};

QueryTrainHandler::QueryTrainHandler(TrainManager& train_manager,
                                     SeatManager& seat_manager)
    : train_manager(train_manager), seat_manager(seat_manager) {}
std::string QueryTrainHandler::execute(const ParamMap& params,
                                       const std::string& timestamp) {
  std::string train_id = params.get('i');
  std::string date_str = params.get('d');
  Date date{std::stoi(date_str.substr(0, 2)), std::stoi(date_str.substr(3))};
  Train train;
  int result = train_manager.queryTrain(train_id, train);
  if (result == -1) {
    return "-1";
  }
  if (!train.is_released) {
    int seats[MAX_STATION_NUM];
    std::fill(seats, seats + train.station_num, train.seat_num);
    return format(train, seats, date);
  }
  if (date > train.sale_date_end || date < train.sale_date_start) {
    return "-1";
  }
  UniTrain unitrain{train_id, date};
  SeatMap seat_map = seat_manager.querySeat(unitrain);
  return format(train, seat_map.seat_num, date);
}

QueryTransferHandler::QueryTransferHandler(TrainManager& train_manager)
    : train_manager(train_manager) {}

std::string QueryTransferHandler::execute(const ParamMap& params,
                                          const std::string& timestamp)