#include "train_command.hpp"

#include "../utilities/parse_by_char.hpp"
#include "command_system.hpp"

AddTrainHandler::AddTrainHandler(TrainManager& manager)
    : train_manager(manager) {}
std::string AddTrainHandler::execute(const ParamMap& params) {
  Train train;
  train.train_id = params.get('i');
  train.station_num = std::stoi(params.get('n'));
  train.seat_num = std::stoi(params.get('m'));
  std::string stations_str = params.get('s');
  parse_by_char(stations_str, '|', train.stations);
  std::string prices_str = params.get('p');
  parse_by_char_accumulate(prices_str, '|', train.prices);
  std::string start_tie_string = params.get('x');
  train.start_time.hour = std::stoi(start_tie_string.substr(0, 2));
  train.start_time.minute = std::stoi(start_tie_string.substr(3));
  std::string travel_times_str = params.get('t');
  parse_by_char(travel_times_str, '|', train.travel_times);
  std::string stop_over_time_str = params.get('o');
  if (stop_over_time_str != "-") {
    parse_by_char(stop_over_time_str, '|', train.stop_over_times);
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
std::string DeleteTrainHandler::execute(const ParamMap& params) {
  std::string train_id = params.get('i');
  int result = train_manager.deleteTrain(train_id);
  return std::to_string(result);
};

ReleaseTrainHandler::ReleaseTrainHandler(TrainManager& manager)
    : train_manager(manager) {}
std::string ReleaseTrainHandler::execute(const ParamMap& params) {
  std::string train_id = params.get('i');
  int result = train_manager.releaseTrain(train_id);
  return std::to_string(result);
};

QueryTrainHandler::QueryTrainHandler(TrainManager& train_manager,
                                     SeatManager& seat_manager)
    : train_manager(train_manager), seat_manager(seat_manager) {}
std::string QueryTrainHandler::execute(const ParamMap& params) {
  std::string train_id = params.get('i');
  std::string date_str = params.get('d');
  Date date{std::stoi(date_str.substr(0, 2)), std::stoi(date_str.substr(3))};
  Train train;
  int result = train_manager.queryTrain(train_id, train);
  if (result == -1) {
    return "-1";
  }
  std::string res = train_id + train.type + '\n';
  if(!train.is_released){
    int seats[MAX_STATION_NUM];
    std::fill(seats, seats + train.station_num, train.seat_num);
    return format(train, seats, date);
  }
  UniTrain unitrain{train_id, date};
  SeatMap seat_map = seat_manager.querySeat(unitrain);
  return format(train, seat_map.seat_num, date);
}