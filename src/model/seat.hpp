
#include "train.hpp"

struct SeatMap{
  int total_seats;
  int station_num;
  int seat_num[MAX_STATION_NUM];

  bool is_seat_available(int start_station, int end_station, int seat){
    for(int i = start_station; i < end_station; i++){
      if(seat_num[i] <= seat){
        return false;
      }
    }
    return true;
  }

  bool book_seat(int start_station, int end_station, int seat){
    if(!is_seat_available(start_station, end_station, seat)){
      return false;
    }
    for(int i = start_station; i < end_station; i++){
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