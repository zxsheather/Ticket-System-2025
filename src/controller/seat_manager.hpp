#pragma once

#include "../model/seat.hpp"
#include "../model/train.hpp"
#include "../storage/memory_river.hpp"

class SeatManager {
 private:
  MemoryRiver<SeatMap> seat_db;

 public:
  SeatManager();
  void initSeat(const Train& train, int& train_seat);
  SeatMap querySeat(int start_pos, int& seat_map_pos, int date_from_sale_start);
  int bookSeat(int seat_map_pos, int start_station, int end_station, int seat,
               SeatMap& seat_map);

  void releaseSeat(int seat_map_pos, int start_station, int end_station,
                   int seat, SeatMap& seat_map);
};