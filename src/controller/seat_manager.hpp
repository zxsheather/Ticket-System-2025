#pragma once
#include <cstdint>

#include "../model/seat.hpp"
#include "../model/train.hpp"
#include "../storage/bplus_tree.hpp"

class SeatManager {
 private:
  //   BPT<UniTrain, SeatMap> seat_db;
  BPT<uint64_t, SeatMap> seat_db;  // Using hashed UniTrain as key
 public:
  SeatManager();
  void initSeat(const Train& train);
  SeatMap querySeat(const FixedString<20>& train_id, const Date& date);
  int bookSeat(const FixedString<20>& train_id, const Date& date,
               int start_station, int end_station, int seat, SeatMap& seat_map);

  void releaseSeat(const FixedString<20>& train_id, const Date& date,
                   int start_station, int end_station, int seat,
                   SeatMap& seat_map);
};