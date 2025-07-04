#include "order_command.hpp"

#include <iostream>

#include "../controller/seat_manager.hpp"
#include "../model/ticket.hpp"
#include "../model/time.hpp"
#include "../utilities/merge_sort.hpp"
#include "command_system.hpp"

QueryTicketHandler::QueryTicketHandler(TrainManager& train_manager,
                                       SeatManager& seat_manager)
    : train_manager(train_manager), seat_manager(seat_manager) {}

void QueryTicketHandler::execute(const ParamMap& params,
                                 const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::string date_str = params.get('d');
  Date date{std::stoi(date_str.substr(0, 2)), std::stoi(date_str.substr(3))};
  std::string start_station = params.get('s');
  std::string end_station = params.get('t');
  sjtu::vector<FixedString<20>> result =
      train_manager.queryRoute({start_station, end_station});

  size_t i = 0, j = 0;

  ComparisonOrder order =
      params.has('p') ? (params.get('p') == "time" ? TIME : COST) : TIME;

  sjtu::vector<TicketInfo> tickets(30);
  int idx = 0;
  sjtu::vector<TicketOrder> ticket_order(30);
  Train train;
  for (auto& train_id : result) {
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
    int pos;
    int seats = seat_manager
                    .querySeat(train.seat_map_pos, pos,
                               origin_date - train.sale_date_start)
                    .queryAvailableSeat(start_index, end_index);
    tickets.push_back(TicketInfo(
        train_id, start_station, end_station,
        TimePoint(origin_date, train.departure_times[start_index]),
        TimePoint(origin_date, train.arrival_times[end_index]), origin_date,
        train.prices[end_index] - train.prices[start_index], seats));

    idx++;
    if (order == TIME) {
      ticket_order.push_back({tickets[idx - 1].minutes, idx - 1, train_id});
    } else {
      ticket_order.push_back({tickets[idx - 1].price, idx - 1, train_id});
    }
  }
  if (idx == 0) {
    std::cout << "0\n";
    return;
  }
  std::cout << idx << '\n';
  mergeSort(ticket_order, 0, idx - 1);
  for (size_t k = 0; k < ticket_order.size(); ++k) {
    int index = ticket_order[k].index;
    std::cout << tickets[index].format() << '\n';
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

void BuyTicketHandler::execute(const ParamMap& params,
                               const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::string username = params.get('u');
  if (user_manager.isLoggedIn(username) == -1) {
    std::cout << "-1\n";
    return;
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
    std::cout << "-1\n";
    return;
  }
  int start_index = train.queryStationIndex(start_station);
  int end_index = train.queryStationIndex(end_station);
  if (start_index == -1 || end_index == -1 || start_index >= end_index) {
    std::cout << "-1\n";
    return;
  }
  Date start_date = date - train.departure_times[start_index].hour / 24;
  if (start_date < train.sale_date_start || start_date > train.sale_date_end) {
    std::cout << "-1\n";
    return;
  }
  int seat_map_pos;
  SeatMap seat_map = seat_manager.querySeat(train.seat_map_pos, seat_map_pos,
                                            start_date - train.sale_date_start);

  if (timestamp == "3514") {
    std::cerr << "SeatMap: ";
    for (int i = 0; i < train.station_num; ++i) {
      std::cerr << seat_map.seat_num[i] << ' ';
    }
  }

  if (ticket_num > train.seat_num) {
    std::cout << "-1\n";
    return;
  }
  int booked = seat_manager.bookSeat(seat_map_pos, start_index, end_index,
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
      std::cout << "queue\n";
    } else {
      std::cout << "-1\n";
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
    std::cout << price * ticket_num << '\n';
  }
}

QueryOrderHandler::QueryOrderHandler(OrderManager& order_manager,
                                     UserManager& user_manager)
    : order_manager(order_manager), user_manager(user_manager) {}

void QueryOrderHandler::execute(const ParamMap& params,
                                const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::string username = params.get('u');
  if (user_manager.isLoggedIn(username) == -1) {
    std::cout << "-1\n";
    return;
  }
  sjtu::vector<Order> orders = order_manager.queryOrder(username);
  if (orders.empty()) {
    std::cout << "0\n";
    return;
  }
  std::cout << orders.size() << '\n';
  for (int i = (int)orders.size() - 1; i >= 0; --i) {
    std::cout << orders[i].format() << '\n';
  }
}

RefundTicketHandler::RefundTicketHandler(OrderManager& order_manager,
                                         UserManager& user_manager,
                                         TrainManager& train_manager,
                                         SeatManager& seat_manager)
    : order_manager(order_manager),
      user_manager(user_manager),
      train_manager(train_manager),
      seat_manager(seat_manager) {}
void RefundTicketHandler::execute(const ParamMap& params,
                                  const std::string& timestamp) {
  std::cout << '[' << timestamp << "] ";
  std::string username = params.get('u');
  if (user_manager.isLoggedIn(username) == -1) {
    std::cout << "-1\n";
    return;
  }
  int order_id = params.has('n') ? std::stoi(params.get('n')) : 1;
  sjtu::vector<Order> orders = order_manager.queryOrder(username);
  if (orders.size() < order_id || order_id <= 0) {
    std::cout << "-1\n";
    return;
  }
  Order& order = orders[orders.size() - order_id];
  if (order.status == REFUNDED) {
    std::cout << "-1\n";
    return;
  }
  if (order.status == PENDING) {
    order_manager.updateOrderStatus(username, order, REFUNDED);
    order_manager.removeFromPending(order.train_id, order.origin_station_date,
                                    order);
    std::cout << "0\n";
    return;
  }
  Train train;
  int result = train_manager.queryTrain(order.train_id, train);
  if (result == -1 || !train.is_released) {
    std::cout << "-1\n";
    return;
  }
  int start_index = train.queryStationIndex(order.from);
  int end_index = train.queryStationIndex(order.to);
  Date date = order.origin_station_date;
  int seat_map_pos;
  SeatMap seat_map = seat_manager.querySeat(train.seat_map_pos, seat_map_pos,
                                            date - train.sale_date_start);
  seat_manager.releaseSeat(seat_map_pos, start_index, end_index,
                           order.ticket_num, seat_map);
  order_manager.updateOrderStatus(username, order, REFUNDED);
  sjtu::vector<Order> pending_orders = order_manager.queryPendingOrder(
      order.train_id, order.origin_station_date);
  if (pending_orders.empty()) {
    std::cout << "0\n";
    return;
  }
  sjtu::vector<Order> need_to_remove;
  for (auto& pending_order : pending_orders) {
    if (pending_order.start_station_index >= end_index ||
        pending_order.end_station_index <= start_index) {
      continue;
    }
    int booked = seat_manager.bookSeat(
        seat_map_pos, pending_order.start_station_index,
        pending_order.end_station_index, pending_order.ticket_num, seat_map);
    if (booked == 0) {
      order_manager.updateOrderStatus(pending_order.username, pending_order,
                                      SUCCESS);
      need_to_remove.push_back(pending_order);
    }
  }
  for (int i = (int)need_to_remove.size() - 1; i >= 0; --i) {
    order_manager.removeFromPending(order.train_id, date, need_to_remove[i]);
  }
  std::cout << "0\n";
}
