#include "seat_manager.hpp"

#include "../model/seat.hpp"

SeatManager::SeatManager() : seat_db("seat.memoryriver") {
  if (!seat_db.exist()) {
    seat_db.initialise();
  }
}

SeatMap SeatManager::querySeat(int start_pos, int& seat_map_pos,
                               int date_from_sale_start) {
  seat_map_pos = start_pos + date_from_sale_start * sizeof(SeatMap);
  SeatMap seat_map;
  seat_db.read(seat_map, seat_map_pos);
  return seat_map;
}

int SeatManager::bookSeat(int seat_map_pos, int start_station, int end_station,
                          int seat, SeatMap& seat_map) {
  if (seat_map.bookSeat(start_station, end_station, seat)) {
    seat_db.update(seat_map, seat_map_pos);
    return 0;
  }
  return -1;
}

void SeatManager::initSeat(const Train& train, int& train_seat) {
  SeatMap seat_map;
  seat_map.total_seats = train.seat_num;
  seat_map.station_num = train.station_num;
  std::fill(seat_map.seat_num, seat_map.seat_num + train.station_num,
            train.seat_num);
  train_seat = seat_db.write(seat_map);
  for (int date = 1; date <= train.sale_date_end - train.sale_date_start;
       date++) {
    seat_db.write(seat_map);
  }
}

void SeatManager::releaseSeat(int seat_map_pos, int start_station,
                              int end_station, int seat, SeatMap& seat_map) {
  seat_map.releaseSeat(start_station, end_station, seat);
  seat_db.update(seat_map, seat_map_pos);
}