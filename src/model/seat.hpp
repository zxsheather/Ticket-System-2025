#pragma once
#include "train.hpp"

struct SeatMap {
  int total_seats;
  int station_num;
  int seat_num[MAX_STATION_NUM];

  int queryAvailableSeat(int start_station, int end_station) {
    int min_seat = seat_num[start_station];
    for (int i = start_station + 1; i < end_station; i++) {
      if (seat_num[i] < min_seat) {
        min_seat = seat_num[i];
      }
    }
    return min_seat;
  }

  bool isSeatAvailable(int start_station, int end_station, int seat) {
    for (int i = start_station; i < end_station; i++) {
      if (seat_num[i] <= seat) {
        return false;
      }
    }
    return true;
  }

  bool bookSeat(int start_station, int end_station, int seat) {
    if (!isSeatAvailable(start_station, end_station, seat)) {
      return false;
    }
    for (int i = start_station; i < end_station; i++) {
      seat_num[i]--;
    }
    return true;
  }

  bool operator<(const SeatMap& other) const {
    return total_seats < other.total_seats;
  }
  bool operator>(const SeatMap& other) const {
    return total_seats > other.total_seats;
  }
  bool operator==(const SeatMap& other) const {
    return total_seats == other.total_seats;
  }
  bool operator!=(const SeatMap& other) const {
    return total_seats != other.total_seats;
  }
  bool operator<=(const SeatMap& other) const {
    return total_seats <= other.total_seats;
  }
  bool operator>=(const SeatMap& other) const {
    return total_seats >= other.total_seats;
  }
};