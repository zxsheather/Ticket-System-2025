#include "train_command.hpp"

#include "../controller/seat_manager.hpp"
#include "../model/time.hpp"
#include "../stl/priority_queue.hpp"
#include "../utilities/parse_by_char.hpp"
#include "command_system.hpp"

enum ComparisonOrder { TIME, COST };

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
  UniTrain unitrain{train_id, date};
  SeatMap seat_map = seat_manager.querySeat(unitrain);
  return format(train, seat_map.seat_num, date);
}

QueryTicketHandler::QueryTicketHandler(TrainManager& train_manager,
                                       SeatManager& seat_manager)
    : train_manager(train_manager), seat_manager(seat_manager) {}

std::string QueryTicketHandler::execute(const ParamMap& params,
                                        const std::string& timestamp) {
  std::string date_str = params.get('d');
  Date date{std::stoi(date_str.substr(0, 2)), std::stoi(date_str.substr(3))};
  std::string start_station = params.get('s');
  std::string end_station = params.get('t');
  sjtu::vector<FixedString<20>> train_ids_from_start =
      train_manager.queryStation(start_station, date);
  sjtu::vector<FixedString<20>> train_ids_from_end =
      train_manager.queryStation(end_station, date);
  sjtu::vector<FixedString<20>> result;
  size_t i = 0, j = 0;
  while (true) {
    if (i >= train_ids_from_start.size() || j >= train_ids_from_end.size()) {
      break;
    }
    if (train_ids_from_start[i] == train_ids_from_end[j]) {
      result.push_back(train_ids_from_start[i]);
      i++;
      j++;
    } else if (train_ids_from_start[i] < train_ids_from_end[j]) {
      i++;
    } else {
      j++;
    }
  }
  ComparisonOrder order =
      params.has('p') ? (params.get('p') == "time" ? TIME : COST) : TIME;

  if (order == TIME) {
    sjtu::priority_queue<TicketInfo, TimeComparatorForQuery> pq;
    for (auto& train_id : result) {
      Train train;
      train_manager.queryTrain(train_id, train);
      int start_index = train.queryStationIndex(start_station);
      int end_index = train.queryStationIndex(end_station);
      if (start_index == -1 || end_index == -1 || start_index >= end_index) {
        continue;
      }
      date = date - train.departure_times[start_index].hour / 24;
      TicketInfo ticket_info(
          train_id, start_station, end_station,
          TimePoint(date, train.departure_times[start_index]),
          TimePoint(date, train.arrival_times[end_index]),
          train.prices[end_index] - train.prices[start_index],
          seat_manager.querySeat(UniTrain(train_id, date))
              .queryAvailableSeat(start_index, end_index));
      pq.push(ticket_info);
    }
    if (pq.empty()) {
      return "0";
    }
    std::string result_str = std::to_string(pq.size()) + "\n";
    while (!pq.empty()) {
      result_str += pq.top().format();
      pq.pop();
      if (!pq.empty()) result_str += "\n";
    }
    return result_str;
  } else {
    sjtu::priority_queue<TicketInfo, CostComparatorForQuery> pq;
    for (auto& train_id : result) {
      Train train;
      train_manager.queryTrain(train_id, train);
      int start_index = train.queryStationIndex(start_station);
      int end_index = train.queryStationIndex(end_station);
      if (start_index == -1 || end_index == -1 || start_index >= end_index) {
        continue;
      }
      date = date - train.departure_times[start_index].hour / 24;
      TicketInfo ticket_info(
          train_id, start_station, end_station,
          TimePoint(date, train.departure_times[start_index]),
          TimePoint(date, train.arrival_times[end_index]),
          train.prices[end_index] - train.prices[start_index],
          seat_manager.querySeat(UniTrain(train_id, date))
              .queryAvailableSeat(start_index, end_index));
      pq.push(ticket_info);
    }
    if (pq.empty()) {
      return "0";
    }
    std::string result_str = std::to_string(pq.size()) + "\n";
    while (!pq.empty()) {
      result_str += pq.top().format();
      pq.pop();
      if (!pq.empty()) result_str += "\n";
    }
    return result_str;
  }
}

BuyTicketHandler::BuyTicketHandler(TrainManager& train_manager,
                                   SeatManager& seat_manager,
                                   UserManager& user_manager,
                                   OrderManager& order_manager)
    : train_manager(train_manager),
      seat_manager(seat_manager),
      user_manager(user_manager),
      order_manager(order_manager) {}

std::string BuyTicketHandler::execute(const ParamMap& params,
                                      const std::string& timestamp) {
  std::string username = params.get('u');
  if (user_manager.isLoggedIn(username) == -1) {
    return "-1";
  }
  std::string train_id = params.get('i');
  std::string date_str = params.get('d');
  Date date{std::stoi(date_str.substr(0, 2)), std::stoi(date_str.substr(3))};
  int ticket_num = std::stoi(params.get('n'));
  std::string start_station = params.get('f');
  std::string end_station = params.get('t');
  bool wait = params.has('q') ? params.get('q') == "true" : false;
  Train train;
  int result = train_manager.queryTrain(train_id, train);
  if (result == -1 || !train.is_released) {
    return "-1";
  }
  if (date < train.sale_date_start || date > train.sale_date_end) {
    return "-1";
  }
  int start_index = train.queryStationIndex(start_station);
  int end_index = train.queryStationIndex(end_station);
  if (start_index == -1 || end_index == -1 || start_index >= end_index) {
    return "-1";
  }
  date = date - train.departure_times[start_index].hour / 24;
  SeatMap seat_map = seat_manager.querySeat(UniTrain(train_id, date));
  if (ticket_num > train.seat_num) {
    return "-1";
  }
  int booked = seat_manager.bookSeat(UniTrain(train_id, date), start_index,
                                     end_index, ticket_num, seat_map);
  if (booked == -1) {
    if (wait) {
      Order order(username, train_id, start_station,
                  TimePoint(date, train.departure_times[start_index]),
                  end_station, TimePoint(date, train.arrival_times[end_index]),
                  ticket_num, std::stoi(timestamp), 0, PENDING);
      order_manager.addOrder(order);
      order_manager.addPendingOrder(order);
      return "queue";
    } else {
      return "-1";
    }
  } else {
    int price = train.prices[end_index] - train.prices[start_index];
    Order order(username, train_id, start_station,
                TimePoint(date, train.departure_times[start_index]),
                end_station, TimePoint(date, train.arrival_times[end_index]),
                ticket_num, std::stoi(timestamp), price, SUCCESS);
    order_manager.addOrder(order);
    return std::to_string(price * ticket_num);
  }
}