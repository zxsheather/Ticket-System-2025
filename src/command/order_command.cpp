#include "order_command.hpp"

#include <iostream>

#include "../controller/seat_manager.hpp"
#include "../model/ticket.hpp"
#include "../model/time.hpp"
#include "../stl/priority_queue.hpp"
#include "command_system.hpp"

QueryTicketHandler::QueryTicketHandler(TrainManager& train_manager,
                                       SeatManager& seat_manager)
    : train_manager(train_manager), seat_manager(seat_manager) {}

std::string QueryTicketHandler::execute(const ParamMap& params,
                                        const std::string& timestamp) {
  std::string date_str = params.get('d');
  Date date{std::stoi(date_str.substr(0, 2)), std::stoi(date_str.substr(3))};
  std::string start_station = params.get('s');
  std::string end_station = params.get('t');
  sjtu::vector<FixedString<20>> result = train_manager.queryRoute(
      {std::move(start_station), std::move(end_station)});

  size_t i = 0, j = 0;

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
      Date origin_date = date - train.departure_times[start_index].hour / 24;
      if (origin_date < train.sale_date_start ||
          origin_date > train.sale_date_end) {
        continue;
      }
      TicketInfo ticket_info(
          train_id, std::move(start_station), std::move(end_station),
          TimePoint(origin_date, train.departure_times[start_index]),
          TimePoint(origin_date, train.arrival_times[end_index]), origin_date,
          train.prices[end_index] - train.prices[start_index],
          seat_manager.querySeat(UniTrain(train_id, origin_date))
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
      Date origin_date = date - train.departure_times[start_index].hour / 24;
      if (origin_date < train.sale_date_start ||
          origin_date > train.sale_date_end) {
        continue;
      }
      TicketInfo ticket_info(
          train_id, std::move(start_station), std::move(end_station),
          TimePoint(origin_date, train.departure_times[start_index]),
          TimePoint(origin_date, train.arrival_times[end_index]), origin_date,
          train.prices[end_index] - train.prices[start_index],
          seat_manager.querySeat(UniTrain(train_id, origin_date))
              .queryAvailableSeat(start_index, end_index));
      pq.push(ticket_info);
      // if(train.train_id.toString() =="LeavesofGrass" && origin_date ==
      // Date{7,2}){
      //   SeatMap seat_map = seat_manager.querySeat(UniTrain(train_id,
      //   origin_date)); std::cerr << '[' << timestamp << ']' << format(train,
      //   seat_map.seat_num, origin_date) << std::endl;
      // }
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
  int start_index = train.queryStationIndex(start_station);
  int end_index = train.queryStationIndex(end_station);
  if (start_index == -1 || end_index == -1 || start_index >= end_index) {
    return "-1";
  }
  Date start_date = date - train.departure_times[start_index].hour / 24;
  if (start_date < train.sale_date_start || start_date > train.sale_date_end) {
    return "-1";
  }
  UniTrain unitrain(train_id, start_date);
  SeatMap seat_map = seat_manager.querySeat(unitrain);
  if (ticket_num > train.seat_num) {
    return "-1";
  }
  int booked = seat_manager.bookSeat(unitrain, start_index, end_index,
                                     ticket_num, seat_map);
  if (booked == -1) {
    if (wait) {
      Order order(std::move(username), std::move(train_id), start_date,
                  start_station, start_index,
                  TimePoint(start_date, train.departure_times[start_index]),
                  end_station, end_index,
                  TimePoint(start_date, train.arrival_times[end_index]),
                  ticket_num, std::stoi(timestamp),
                  train.prices[end_index] - train.prices[start_index], PENDING);
      order_manager.addOrder(order);
      order_manager.addPendingOrder(order);
      return "queue";
    } else {
      return "-1";
    }
  } else {
    int price = train.prices[end_index] - train.prices[start_index];
    Order order(std::move(username), std::move(train_id), start_date,
                start_station, start_index,
                TimePoint(start_date, train.departure_times[start_index]),
                end_station, end_index,
                TimePoint(start_date, train.arrival_times[end_index]),
                ticket_num, std::stoi(timestamp), price, SUCCESS);
    order_manager.addOrder(order);
    // if(train.train_id.toString() =="LeavesofGrass" && start_date ==
    // Date{7,2}){
    //   SeatMap seat_map = seat_manager.querySeat(UniTrain(train_id,
    //   start_date)); std::cerr << '[' << timestamp << ']' << format(train,
    //   seat_map.seat_num, start_date) << std::endl;
    // }
    return std::to_string(price * ticket_num);
  }
}

QueryOrderHandler::QueryOrderHandler(OrderManager& order_manager,
                                     UserManager& user_manager)
    : order_manager(order_manager), user_manager(user_manager) {}

std::string QueryOrderHandler::execute(const ParamMap& params,
                                       const std::string& timestamp) {
  std::string username = params.get('u');
  if (user_manager.isLoggedIn(username) == -1) {
    return "-1";
  }
  sjtu::vector<Order> orders = order_manager.queryOrder(username);
  if (orders.empty()) {
    return "0";
  }
  std::string result;
  result += std::to_string(orders.size()) + "\n";
  for (size_t i = orders.size() - 1; i > 0; --i) {
    result += orders[i].format() + "\n";
  }
  result += orders[0].format();
  return result;
}

RefundTicketHandler::RefundTicketHandler(OrderManager& order_manager,
                                         UserManager& user_manager,
                                         TrainManager& train_manager,
                                         SeatManager& seat_manager)
    : order_manager(order_manager),
      user_manager(user_manager),
      train_manager(train_manager),
      seat_manager(seat_manager) {}
std::string RefundTicketHandler::execute(const ParamMap& params,
                                         const std::string& timestamp) {
  std::string username = params.get('u');
  if (user_manager.isLoggedIn(username) == -1) {
    return "-1";
  }
  int order_id = params.has('n') ? std::stoi(params.get('n')) : 1;
  sjtu::vector<Order> orders = order_manager.queryOrder(username);
  if (orders.size() < order_id || order_id <= 0) {
    return "-1";
  }
  Order& order = orders[orders.size() - order_id];
  // std::cerr << '[' << timestamp << ']' << username << std::endl;
  // for(auto& o : orders) {
  //   std::cerr << o.timestamp<<o.format() << std::endl;
  // }
  if (order.status == REFUNDED) {
    return "-1";
  }
  if (order.status == PENDING) {
    order_manager.updateOrderStatus(username, order, REFUNDED);
    order_manager.removeFromPending(
        UniTrain(order.train_id, order.origin_station_date), order);
    return "0";
  }
  Train train;
  int result = train_manager.queryTrain(order.train_id, train);
  if (result == -1 || !train.is_released) {
    return "-1";
  }
  int start_index = train.queryStationIndex(order.from);
  int end_index = train.queryStationIndex(order.to);
  Date date = order.origin_station_date;
  UniTrain unitrain(order.train_id, date);
  SeatMap seat_map = seat_manager.querySeat(unitrain);
  seat_manager.releaseSeat(unitrain, start_index, end_index, order.ticket_num,
                           seat_map);
  order_manager.updateOrderStatus(username, order, REFUNDED);
  sjtu::vector<Order> pending_orders =
      order_manager.queryPendingOrder(unitrain);
  if (pending_orders.empty()) {
    return "0";
  }
  // if(train.train_id.toString() == "LeavesofGrass" && date == Date{7,2}) {
  //     std::cerr << '[' << timestamp << ']' << format(train,
  //     seat_map.seat_num, date) << std::endl;
  // }
  sjtu::vector<Order> need_to_remove;
  for (auto& pending_order : pending_orders) {
    if (pending_order.start_station_index >= end_index ||
        pending_order.end_station_index <= start_index) {
      continue;
    }
    int booked = seat_manager.bookSeat(
        unitrain, pending_order.start_station_index,
        pending_order.end_station_index, pending_order.ticket_num, seat_map);
    if (booked == 0) {
      order_manager.updateOrderStatus(pending_order.username, pending_order,
                                      SUCCESS);
      need_to_remove.push_back(pending_order);
      // if(train.train_id.toString() == "LeavesofGrass" && date == Date{7,2}) {
      //   std::cerr << '[' << timestamp << ']' << format(train,
      //   seat_map.seat_num, date) << std::endl;
      // }
    }
  }
  for (int i = (int)need_to_remove.size() - 1; i >= 0; --i) {
    order_manager.removeFromPending(unitrain, need_to_remove[i]);
  }
  return "0";
}
