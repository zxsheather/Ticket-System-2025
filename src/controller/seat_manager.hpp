#pragma once
#include "../model/seat.hpp"
#include "../model/train.hpp"
#include "../storage/bplus_tree.hpp"

class SeatManager {
 private:
  BPT<UniTrain, SeatMap> seat_db;

 public:
  SeatManager();
  SeatMap querySeat(const UniTrain& unitrain);
};