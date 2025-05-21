#pragma once

#include "../utilities/limited_sized_string.hpp"

constexpr int MAX_STATION_NUM = 26;
constexpr int days_in_month[12] = {31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31};
struct Date {
  int month{}, day{};
  std::string toString() const {
    std::string result;
    if (month < 10) result += '0';
    result += std::to_string(month) + '-';
    if (day < 10) result += '0';
    result += std::to_string(day);
    return result;
  }
};

struct Time {
  int hour{}, minute{};
  std::string toString() const {
    std::string result;
    if (hour < 10) result += '0';
    result += std::to_string(hour) + ':';
    if (minute < 10) result += '0';
    result += std::to_string(minute);
    return result;
  }
};

struct TimePoint {
  Date date{};
  Time time{};

  TimePoint() = default;

  std::string toString() const {
    return date.toString() + ' ' + time.toString();
  }

  bool operator<(const TimePoint& other) const {
    if (date.month != other.date.month) return date.month < other.date.month;
    if (date.day != other.date.day) return date.day < other.date.day;
    if (time.hour != other.time.hour) return time.hour < other.time.hour;
    return time.minute < other.time.minute;
  }
  bool operator>(const TimePoint& other) const {
    if (date.month != other.date.month) return date.month > other.date.month;
    if (date.day != other.date.day) return date.day > other.date.day;
    if (time.hour != other.time.hour) return time.hour > other.time.hour;
    return time.minute > other.time.minute;
  }
  bool operator==(const TimePoint& other) const {
    return date.month == other.date.month && date.day == other.date.day &&
           time.hour == other.time.hour && time.minute == other.time.minute;
  }
  bool operator!=(const TimePoint& other) const { return !(*this == other); }
  bool operator<=(const TimePoint& other) const {
    return *this < other || *this == other;
  }
  bool operator>=(const TimePoint& other) const {
    return *this > other || *this == other;
  }

  TimePoint(const Date& date, const Time& time) : date(date), time(time) {}

  TimePoint operator+(const Time& other) const {
    TimePoint result = *this;
    result.time.hour += other.hour;
    result.time.minute += other.minute;
    if (result.time.minute >= 60) {
      result.time.hour++;
      result.time.minute -= 60;
    }
    if (result.time.hour >= 24) {
      result.date.day++;
      result.time.hour -= 24;
    }
    while (result.date.day > days_in_month[result.date.month - 1]) {
      result.date.day -= days_in_month[result.date.month - 1];
      result.date.month++;
    }
    return result;
  }
  TimePoint operator+(int minutes) const {
    TimePoint result = *this;
    result.time.minute += minutes;
    if (result.time.minute >= 60) {
      result.time.hour += result.time.minute / 60;
      result.time.minute %= 60;
    }
    if (result.time.hour >= 24) {
      result.date.day += result.time.hour / 24;
      result.time.hour %= 24;
    }
    while (result.date.day > days_in_month[result.date.month - 1]) {
      result.date.day -= days_in_month[result.date.month - 1];
      result.date.month++;
    }
    return result;
  }
};

struct Train {
  FixedString<20> train_id{};
  int station_num{};
  FixedString<30> stations[MAX_STATION_NUM]{};
  int seat_num{};
  int prices[MAX_STATION_NUM - 1]{};
  Time start_time{};
  int travel_times[MAX_STATION_NUM - 1]{};
  int stop_over_times[MAX_STATION_NUM - 2]{};
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

  void getTimeAll(TimePoint* arrival_time, TimePoint* departure_time,
                  const Date& departure_day) const {
    departure_time[0] = {departure_day, start_time};
    for (int i = 1; i < station_num - 1; i++) {
      arrival_time[i] = departure_time[i - 1] + travel_times[i - 1];
      departure_time[i] = arrival_time[i] + stop_over_times[i - 1];
    }
    arrival_time[station_num - 1] =
        departure_time[station_num - 2] + travel_times[station_num - 2];
  }
};

struct UniTrain {
  FixedString<20> train_id{};
  Date date{};

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
