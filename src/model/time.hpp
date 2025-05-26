#pragma once
#include <string>
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
  Date operator+(int days) const {
    Date result = *this;
    result.day += days;
    while (result.day > days_in_month[result.month - 1]) {
      result.day -= days_in_month[result.month - 1];
      result.month++;
      if (result.month > 12) {
        result.month = 1;
      }
    }
    return result;
  }
  Date operator-(int days) const {
    Date result = *this;
    result.day -= days;
    while (result.day <= 0) {
      result.month--;
      if (result.month <= 0) {
        result.month = 12;
      }
      result.day += days_in_month[result.month - 1];
    }
    return result;
  }

  bool operator<(const Date& other) const {
    if (month != other.month) return month < other.month;
    return day < other.day;
  }
  bool operator>(const Date& other) const {
    if (month != other.month) return month > other.month;
    return day > other.day;
  }
  bool operator==(const Date& other) const {
    return month == other.month && day == other.day;
  }
  bool operator!=(const Date& other) const { return !(*this == other); }
  bool operator<=(const Date& other) const {
    return *this < other || *this == other;
  }
  bool operator>=(const Date& other) const {
    return *this > other || *this == other;
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
  Time operator+(int minutes) const {
    Time result = *this;
    result.minute += minutes;
    if (result.minute >= 60) {
      result.hour += result.minute / 60;
      result.minute %= 60;
    }
    return result;
  }
  Time operator+(const Time& other) const {
    Time result = *this;
    result.hour += other.hour;
    result.minute += other.minute;
    if (result.minute >= 60) {
      result.hour += result.minute / 60;
      result.minute %= 60;
    }
    return result;
  }
  Time operator+=(const Time& other) {
    hour += other.hour;
    minute += other.minute;
    if (minute >= 60) {
      hour += minute / 60;
      minute %= 60;
    }
    return *this;
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

  TimePoint(const Date& date, const Time& time) {
    this->date = date + time.hour / 24;
    this->time.hour = time.hour % 24;
    this->time.minute = time.minute;
  }

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
  int operator-(const TimePoint& other) const {
    int days = 0;
    if (date.month != other.date.month) {
      days += (date.month - other.date.month) * 30;
    }
    if (date.day != other.date.day) {
      days += date.day - other.date.day;
    }
    return days * 24 * 60 + (time.hour - other.time.hour) * 60 +
           (time.minute - other.time.minute);
  }
};