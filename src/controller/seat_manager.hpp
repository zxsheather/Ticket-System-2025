#include "../model/train.hpp"
#include "../storage/bplus_tree.hpp"
#include "../model/seat.hpp"

class SeatManager {
 private:
  BPT<UniTrain, SeatMap> seat_db;
 public:
  SeatManager();
  SeatMap querySeat(const UniTrain& unitrain);
};