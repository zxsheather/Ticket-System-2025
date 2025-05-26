#pragma once

#include "../utilities/limited_sized_string.hpp"
#include "time.hpp"

struct TicketInfo {
  FixedString<20> train_id{};
  FixedString<30> from{};
  FixedString<30> to{};
  TimePoint start_time{};
  TimePoint end_time{};
  int price{};
  int seats{};
  int minutes{};

  TicketInfo() = default;
  TicketInfo(const FixedString<20>& train_id, const FixedString<30>& from,
             const FixedString<30>& to, const TimePoint& start_time,
             const TimePoint& end_time, int price, int seats)
      : train_id(train_id),
        from(from),
        to(to),
        start_time(start_time),
        end_time(end_time),
        price(price),
        seats(seats),
        minutes(end_time - start_time) {}

  TicketInfo(const FixedString<20>& train_id, const std::string& from,
             const std::string& to, const TimePoint& start_time,
             const TimePoint& end_time, int price, int seats)
      : train_id(train_id),
        from(from),
        to(to),
        start_time(start_time),
        end_time(end_time),
        price(price),
        seats(seats),
        minutes(end_time - start_time) {}

  std::string format() const {
    std::string result = train_id.toString() + ' ' + from.toString() + ' ' +
                         start_time.toString() + " -> " + to.toString() + ' ' +
                         end_time.toString() + ' ' + std::to_string(price) +
                         ' ' + std::to_string(seats);
    return result;
  }
};
