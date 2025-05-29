#include "seat_manager.hpp"

#include <cstdint>

#include "../model/seat.hpp"
#include "../utilities/hash.hpp"

SeatManager::SeatManager() : seat_db("seat") {}

SeatMap SeatManager::querySeat(const FixedString<20>& train_id,
                               const Date& date) {
  uint64_t hashed_key = Hash::hashKey(train_id, date);
  sjtu::vector<SeatMap> seat_maps = seat_db.find(hashed_key);
  return seat_maps[0];
}

int SeatManager::bookSeat(const FixedString<20>& train_id, const Date& date,
                          int start_station, int end_station, int seat,
                          SeatMap& seat_map) {
  if (seat_map.bookSeat(start_station, end_station, seat)) {
    uint64_t hashed_key = Hash::hashKey(train_id, date);
    seat_db.update(hashed_key, seat_map);
    return 0;
  }
  return -1;
}

void SeatManager::initSeat(const Train& train) {
  // if(train.train_id.toString() == "LeavesofGrass") {
  //   std::cerr << "Initializing seat for train: " << train.train_id.toString()
  //             << " with " << train.seat_num << " seats." << std::endl;
  // }
  SeatMap seat_map;
  for (Date date = train.sale_date_start; date <= train.sale_date_end;
       date = date + 1) {
    seat_map.total_seats = train.seat_num;
    seat_map.station_num = train.station_num;
    std::fill(seat_map.seat_num, seat_map.seat_num + train.station_num,
              train.seat_num);
    uint64_t hashed_key = Hash::hashKey(train.train_id, date);
    seat_db.insert(hashed_key, seat_map);
  }
}

void SeatManager::releaseSeat(const FixedString<20>& train_id, const Date& date,
                              int start_station, int end_station, int seat,
                              SeatMap& seat_map) {
  // if(unitrain.train_id.toString() == "LeavesofGrass" && unitrain.date ==
  // Date{7,2}) {
  //   std::cerr << "Releasing seat for train: " << unitrain.train_id.toString()
  //             << " from station " << start_station << " to station "
  //             << end_station << " for " << seat << " seats on " <<
  //             unitrain.date.toString() << std::endl;
  // }
  seat_map.releaseSeat(start_station, end_station, seat);
  uint64_t hashed_key = Hash::hashKey(train_id, date);
  seat_db.update(hashed_key, seat_map);
}