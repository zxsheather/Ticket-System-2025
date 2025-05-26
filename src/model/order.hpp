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
  FixedString<30> from{};
  TimePoint start_time{};
  FixedString<30> to{};
  TimePoint end_time{};
  int ticket_num{};
  int timestamp{};
  int price{};
  OrderStatus status{PENDING};

  Order() = default;
  Order(const std::string& username, const std::string& train_id,
        const std::string& from, const TimePoint& start_time,
        const std::string& to, const TimePoint& end_time, int ticket_num,
        int timestamp, int price, OrderStatus status = PENDING)
      : username(username),
        train_id(train_id),
        from(from),
        start_time(start_time),
        to(to),
        end_time(end_time),
        ticket_num(ticket_num),
        timestamp(timestamp),
        price(price),
        status(status) {}

  Order(const FixedString<20>& username, const FixedString<20>& train_id,
        const FixedString<30>& from, const TimePoint& start_time,
        const FixedString<30>& to, const TimePoint& end_time, int ticket_num,
        int timestamp, int price, OrderStatus status = PENDING)
      : username(username),
        train_id(train_id),
        from(from),
        start_time(start_time),
        to(to),
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
        result += "success ";
        break;
      case PENDING:
        result += "pending ";
        break;
      case REFUNDED:
        result += "refunded ";
        break;
    }
    result += train_id.toString() + ' ' + from.toString() + ' ' +
              start_time.toString() + " -> " + to.toString() + ' ' +
              end_time.toString() + ' ' + std::to_string(price) + ' ' +
              std::to_string(ticket_num) + '\n';
    return result;
  }
};
