#include "seat_manager.hpp"

#include "../model/seat.hpp"

SeatManager::SeatManager() : seat_db("seat") {}

SeatMap SeatManager::querySeat(const UniTrain& unitrain) {
  return seat_db.find(unitrain)[0];
}

int SeatManager::bookSeat(const UniTrain& unitrain, int start_station,
                          int end_station, int seat, SeatMap& seat_map) {
  if (seat_map.bookSeat(start_station, end_station, seat)) {
    seat_db.update(unitrain, seat_map);
    return 0;
  }
  return -1;
}

void SeatManager::initSeat(const Train& train) {
  SeatMap seat_map;
  for (Date date = train.sale_date_start; date <= train.sale_date_end;
       date = date + 1) {
    seat_map.total_seats = train.seat_num;
    seat_map.station_num = train.station_num;
    std::fill(seat_map.seat_num, seat_map.seat_num + train.station_num,
              train.seat_num);
    seat_db.insert(UniTrain(train.train_id, date), seat_map);
  }
}

void SeatManager::releaseSeat(const UniTrain& unitrain, int start_station,
                              int end_station, int seat, SeatMap& seat_map) {
  seat_map.releaseSeat(start_station, end_station, seat);
  seat_db.update(unitrain, seat_map);
}