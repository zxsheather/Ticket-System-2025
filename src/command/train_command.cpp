#include "train_command.hpp"

#include <iostream>

#include "../controller/seat_manager.hpp"
#include "../model/ticket.hpp"
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
  // for debugging
  // if(train.train_id.toString() == "LeavesofGrass") {
  //   std::cerr << train.format() << std::endl;
  // }

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
  if (date < train.sale_date_start || date > train.sale_date_end) {
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

QueryTransferHandler::QueryTransferHandler(TrainManager& train_manager,
                                           SeatManager& seat_manager)
    : train_manager(train_manager), seat_manager(seat_manager) {}

std::string QueryTransferHandler::execute(const ParamMap& params,
                                          const std::string& timestamp) {
  std::string start_station = params.get('s');
  std::string end_station = params.get('t');
  std::string date_str = params.get('d');
  bool is_time = params.has('p') ? params.get('p') == "time" : true;
  Date date{std::stoi(date_str.substr(0, 2)), std::stoi(date_str.substr(3))};
  sjtu::vector<FixedString<20>> train_ids_from_start =
      train_manager.queryStation(start_station);
  sjtu::vector<FixedString<20>> train_ids_from_end =
      train_manager.queryStation(end_station);
  sjtu::vector<Train> trains_to_end;
  for (const auto& train_id : train_ids_from_end) {
    Train train;
    train_manager.queryTrain(train_id, train);
    // if (train.sale_date_start > date + 6 || train.sale_date_end < date - 3) {
    //   continue;
    // }
    trains_to_end.push_back(train);
  }
  sjtu::vector<int> end_station_indices;
  sjtu::vector<sjtu::map<FixedString<30>, int>> stations_valid_from_end;
  for (auto& train : trains_to_end) {
    sjtu::map<FixedString<30>, int> valid_stations;
    int end_index = train.queryStationIndex(end_station);
    for (int i = 0; i < end_index; ++i) {
      valid_stations[train.stations[i]] = i;
    }
    end_station_indices.push_back(end_index);
    stations_valid_from_end.push_back(valid_stations);
  }
  int min_price = 0x3f3f3f3f;
  int min_time = 0x3f3f3f3f;
  TicketInfo ticket1, ticket2;
  int final_start_index, final_transfer_index_from_start,
      final_transfer_index_from_end, final_end_index;
  for (auto& train_id : train_ids_from_start) {
    Train train;
    train_manager.queryTrain(train_id, train);
    int start_index = train.queryStationIndex(start_station);
    if (start_index == -1 ||
        train.sale_date_start + train.departure_times[start_index].hour / 24 >
            date ||
        train.sale_date_end + train.departure_times[start_index].hour / 24 <
            date) {
      continue;
    }
    Date origin_date1 = date - train.departure_times[start_index].hour / 24;
    TimePoint start_time(origin_date1, train.departure_times[start_index]);
    for (size_t i = start_index + 1; i < train.station_num; ++i) {
      TimePoint arrival_time(origin_date1, train.arrival_times[i]);
      for (size_t j = 0; j < trains_to_end.size(); ++j) {
        auto& end_train = trains_to_end[j];
        if (end_train.train_id == train.train_id) {
          continue;
        }
        auto iter = stations_valid_from_end[j].find(train.stations[i]);
        if (iter == stations_valid_from_end[j].end()) {
          continue;
        }
        int end_transfer_index = iter->second;
        TimePoint sale_date_start_timepoint(
            end_train.sale_date_start,
            end_train.departure_times[end_transfer_index]);
        TimePoint sale_date_end_timepoint(
            end_train.sale_date_end,
            end_train.departure_times[end_transfer_index]);
        if (sale_date_end_timepoint < arrival_time) {
          continue;
        }
        TimePoint departure_from_transfer;
        TimePoint arrive_at_end_station;
        Date origin_date2;
        if (sale_date_start_timepoint >= arrival_time) {
          origin_date2 = end_train.sale_date_start;
        } else {
          Time transfer_departure_point(
              end_train.departure_times[end_transfer_index].hour % 24,
              end_train.departure_times[end_transfer_index].minute);
          if (arrival_time.time <= transfer_departure_point) {
            origin_date2 =
                arrival_time.date -
                end_train.departure_times[end_transfer_index].hour / 24;
          } else {
            origin_date2 =
                arrival_time.date -
                end_train.departure_times[end_transfer_index].hour / 24 + 1;
          }
        }
        departure_from_transfer = TimePoint(
            origin_date2, end_train.departure_times[end_transfer_index]);
        arrive_at_end_station = TimePoint(
            origin_date2, end_train.arrival_times[end_station_indices[j]]);
        int travel_time = arrive_at_end_station - start_time;
        int travel_price = end_train.prices[end_station_indices[j]] -
                           end_train.prices[end_transfer_index] +
                           train.prices[i] - train.prices[start_index];
        bool time_minor =
            travel_time < min_time ||
            (travel_time == min_time && travel_price < min_price) ||
            (travel_time == min_time && travel_price == min_price &&
             ticket1.train_id > train.train_id);
        bool price_minor =
            travel_price < min_price ||
            (travel_price == min_price && travel_time < min_time) ||
            (travel_time == min_time && travel_price == min_price &&
             ticket1.train_id > train.train_id);
        if ((is_time && time_minor) || (!is_time && price_minor)) {
          min_price = travel_price;
          min_time = travel_time;
          final_start_index = start_index;
          final_end_index = end_station_indices[j];
          final_transfer_index_from_start = i;
          final_transfer_index_from_end = end_transfer_index;
          ticket1 = TicketInfo(train.train_id, start_station, train.stations[i],
                               start_time, arrival_time, origin_date1,
                               train.prices[i] - train.prices[start_index], 1);
          ticket2 = TicketInfo(end_train.train_id, train.stations[i],
                               end_station, departure_from_transfer,
                               arrive_at_end_station, origin_date2,
                               end_train.prices[end_station_indices[j]] -
                                   end_train.prices[end_transfer_index],
                               1);
        }
      }
    }
  }
  if (min_time == 0x3f3f3f3f) {
    return "0";
  }
  SeatMap seat_map1 =
      seat_manager.querySeat(UniTrain(ticket1.train_id, ticket1.origin_date));
  SeatMap seat_map2 =
      seat_manager.querySeat(UniTrain(ticket2.train_id, ticket2.origin_date));
  ticket1.seats = seat_map1.queryAvailableSeat(final_start_index,
                                               final_transfer_index_from_start);
  ticket2.seats = seat_map2.queryAvailableSeat(final_transfer_index_from_end,
                                               final_end_index);
  return ticket1.format() + '\n' + ticket2.format();
}