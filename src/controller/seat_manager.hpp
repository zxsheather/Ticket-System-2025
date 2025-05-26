#pragma once
#include "../model/seat.hpp"
#include "../model/train.hpp"
#include "../storage/bplus_tree.hpp"

class SeatManager {
 private:
  BPT<UniTrain, SeatMap> seat_db;

 public:
  SeatManager();
  void initSeat(const Train& train);
  SeatMap querySeat(const UniTrain& unitrain);
  int bookSeat(const UniTrain& unitrain, int start_station, int end_station,
               int seat, SeatMap& seat_map);

  void releaseSeat(const UniTrain& unitrain, int start_station, int end_station,
                   int seat, SeatMap& seat_map);
};