#pragma once
#include "../utilities/limited_sized_string.hpp"
#include "time.hpp"

// struct PendingOrder {
//   FixedString<20> train_id{};
//   int start_station{};
//   int end_station{};
//   int ticket_num{};
//   int timestamp{};

//   bool operator<(const PendingOrder& other) const {
//     return timestamp < other.timestamp;
//   }
//   bool operator>(const PendingOrder& other) const {
//     return timestamp > other.timestamp;
//   }
//   bool operator==(const PendingOrder& other) const {
//     return timestamp == other.timestamp;
//   }
//   bool operator!=(const PendingOrder& other) const {
//     return timestamp != other.timestamp;
//   }
//   bool operator<=(const PendingOrder& other) const {
//     return timestamp <= other.timestamp;
//   }
//   bool operator>=(const PendingOrder& other) const {
//     return timestamp >= other.timestamp;
//   }
// };
enum OrderStatus { SUCCESS = 0, PENDING = 1, REFUNDED = 2 };

struct Order {
  FixedString<20> username{};
  FixedString<20> train_id{};
  Date origin_station_date{};
  FixedString<30> from{};
  int start_station_index{};
  TimePoint start_time{};
  FixedString<30> to{};
  int end_station_index{};
  TimePoint end_time{};
  int ticket_num{};
  int timestamp{};
  int price{};
  OrderStatus status{PENDING};

  Order() = default;

  Order(const std::string& username, const std::string& train_id,
        const Date& origin_station_date, const std::string& from,
        int start_station_index, const TimePoint& start_time,
        const std::string& to, int end_station_index, const TimePoint& end_time,
        int ticket_num, int timestamp, int price, OrderStatus status = PENDING)
      : username(username),
        train_id(train_id),
        origin_station_date(origin_station_date),
        from(from),
        start_station_index(start_station_index),
        start_time(start_time),
        to(to),
        end_station_index(end_station_index),
        end_time(end_time),
        ticket_num(ticket_num),
        timestamp(timestamp),
        price(price),
        status(status) {}

  Order(const FixedString<20>& username, const FixedString<20>& train_id,
        const Date& origin_station_date, const FixedString<30>& from,
        int start_station_index, const TimePoint& start_time,
        const FixedString<30>& to, int end_station_index,
        const TimePoint& end_time, int ticket_num, int timestamp, int price,
        OrderStatus status = PENDING)
      : username(username),
        train_id(train_id),
        origin_station_date(origin_station_date),
        from(from),
        start_station_index(start_station_index),
        start_time(start_time),
        to(to),
        end_station_index(end_station_index),
        end_time(end_time),
        ticket_num(ticket_num),
        timestamp(timestamp),
        price(price),
        status(status) {}

  bool operator<(const Order& other) const {
    return timestamp < other.timestamp;
  }
  bool operator>(const Order& other) const {
    return timestamp > other.timestamp;
  }
  bool operator==(const Order& other) const {
    return timestamp == other.timestamp;
  }
  bool operator!=(const Order& other) const {
    return timestamp != other.timestamp;
  }
  bool operator<=(const Order& other) const {
    return timestamp <= other.timestamp;
  }
  bool operator>=(const Order& other) const {
    return timestamp >= other.timestamp;
  }

  std::string format() const {
    std::string result;
    switch (status) {
      case SUCCESS:
        result += "[success] ";
        break;
      case PENDING:
        result += "[pending] ";
        break;
      case REFUNDED:
        result += "[refunded] ";
        break;
    }
    result += train_id.toString() + ' ' + from.toString() + ' ' +
              start_time.toString() + " -> " + to.toString() + ' ' +
              end_time.toString() + ' ' + std::to_string(price) + ' ' +
              std::to_string(ticket_num);
    return result;
  }
};
