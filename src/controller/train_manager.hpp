#pragma once
#include "../model/station.hpp"
#include "../model/train.hpp"
#include "../storage/bplus_tree.hpp"

class TrainManager {
 private:
  BPT<long long, Train> train_db;
  BPT<FixedString<30>, FixedString<20>>
      station_db;  // Maps station to train IDs

 public:
  TrainManager();

  int addTrain(const Train& train);

  int deleteTrain(const std::string& train_id);

  int releaseTrain(const std::string& train_id, Train& train);

  int queryTrain(const std::string& train_id, Train& train);

  int queryTrain(const FixedString<20>& train_id, Train& train);

  sjtu::vector<FixedString<20>> queryStation(const std::string& station_id);
};
