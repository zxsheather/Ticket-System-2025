#pragma once

#include "../utilities/limited_sized_string.hpp"
#include "time.hpp"

constexpr int MAX_STATION_NUM = 26;

struct Train {
  FixedString<20> train_id{};
  int station_num{};
  FixedString<30> stations[MAX_STATION_NUM]{};
  int seat_num{};
  int prices[MAX_STATION_NUM]{};
  // Time start_time{};
  //   int travel_times[MAX_STATION_NUM - 1]{};
  //   int stop_over_times[MAX_STATION_NUM - 2]{};
  Time arrival_times[MAX_STATION_NUM]{};
  Time departure_times[MAX_STATION_NUM]{};
  Date sale_date_start{};
  Date sale_date_end{};
  char type{};
  bool is_released{false};

  bool operator<(const Train& other) const { return train_id < other.train_id; }
  bool operator>(const Train& other) const { return train_id > other.train_id; }
  bool operator==(const Train& other) const {
    return train_id == other.train_id;
  }
  bool operator!=(const Train& other) const {
    return train_id != other.train_id;
  }
  bool operator<=(const Train& other) const {
    return train_id <= other.train_id;
  }
  bool operator>=(const Train& other) const {
    return train_id >= other.train_id;
  }

  int queryStationIndex(const std::string& station_id) const {
    for (int i = 0; i < station_num; ++i) {
      if (stations[i].toString() == station_id) {
        return i;
      }
    }
    return -1;
  }
  int queryStationIndex(const FixedString<30>& station_id) const {
    for (int i = 0; i < station_num; ++i) {
      if (stations[i] == station_id) {
        return i;
      }
    }
    return -1;
  }
};

struct UniTrain {
  FixedString<20> train_id{};
  Date date{};

  UniTrain() = default;
  UniTrain(const std::string& train_id, const Date& date)
      : train_id(train_id), date(date) {}

  UniTrain(const FixedString<20>& train_id, const Date& date)
      : train_id(train_id), date(date) {}

  bool operator<(const UniTrain& other) const {
    if (train_id != other.train_id) return train_id < other.train_id;
    if (date.month != other.date.month) return date.month < other.date.month;
    return date.day < other.date.day;
  }
  bool operator>(const UniTrain& other) const {
    if (train_id != other.train_id) return train_id > other.train_id;
    if (date.month != other.date.month) return date.month > other.date.month;
    return date.day > other.date.day;
  }
  bool operator==(const UniTrain& other) const {
    return train_id == other.train_id && date.month == other.date.month &&
           date.day == other.date.day;
  }
  bool operator!=(const UniTrain& other) const { return !(*this == other); }
  bool operator<=(const UniTrain& other) const {
    return *this < other || *this == other;
  }
  bool operator>=(const UniTrain& other) const {
    return *this > other || *this == other;
  }
};
