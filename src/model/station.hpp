#pragma once
#include "../utilities/limited_sized_string.hpp"
#include "time.hpp"

struct UniStation {
  FixedString<30> station_id{};
  Date date{};

  UniStation() = default;
  UniStation(const FixedString<30>& station_id, const Date& date)
      : station_id(station_id), date(date) {}
  UniStation(const std::string& station_id, const Date& date)
      : station_id(station_id), date(date) {}

  bool operator<(const UniStation& other) const {
    if (station_id < other.station_id) return true;
    if (station_id > other.station_id) return false;
    return date < other.date;
  }
  bool operator>(const UniStation& other) const {
    if (station_id > other.station_id) return true;
    if (station_id < other.station_id) return false;
    return date > other.date;
  }
  bool operator==(const UniStation& other) const {
    return station_id == other.station_id && date == other.date;
  }
  bool operator!=(const UniStation& other) const { return !(*this == other); }
  bool operator<=(const UniStation& other) const {
    return *this < other || *this == other;
  }
  bool operator>=(const UniStation& other) const {
    return *this > other || *this == other;
  }
};
