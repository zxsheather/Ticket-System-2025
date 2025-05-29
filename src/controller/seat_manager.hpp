#pragma once

#include "../model/seat.hpp"
#include "../model/train.hpp"
#include "../storage/bplus_tree.hpp"

class SeatManager {
 private:
  //   BPT<UniTrain, SeatMap> seat_db;
  //   BPT<uint64_t, SeatMap> seat_db;  // Using hashed UniTrain as key
  MemoryRiver<SeatMap> seat_db;
  BPT<FixedString<20>, int> train_seat_pos;  // train_id -> position in seat_db
 public:
  SeatManager();
  void initSeat(const Train& train);
  SeatMap querySeat(const FixedString<20>& train_id, int& seat_map_pos,
                    int date_from_sale_start);
  int bookSeat(int seat_map_pos, int start_station, int end_station, int seat,
               SeatMap& seat_map);

  void releaseSeat(int seat_map_pos, int start_station, int end_station,
                   int seat, SeatMap& seat_map);

  int queryStartPos(const FixedString<20>& train_id) {
    sjtu::vector<int> pos = train_seat_pos.find(train_id);
    if (pos.empty()) {
      return -1;  // Train not found
    }
    return pos[0];
  }
};