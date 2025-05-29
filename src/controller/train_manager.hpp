#pragma once

#include "../model/train.hpp"
#include "../storage/bplus_tree.hpp"

class TrainManager {
 private:
  BPT<FixedString<20>, Train> train_db;
  BPT<FixedString<30>, FixedString<20>>
      station_db;                        // Maps station to train IDs
  BPT<Route, FixedString<20>> route_db;  // Maps route to train IDs
 public:
  TrainManager();

  int addTrain(const Train& train);

  int deleteTrain(const std::string& train_id);

  int releaseTrain(const std::string& train_id, Train& train);

  int queryTrain(const std::string& train_id, Train& train);

  int queryTrain(const FixedString<20>& train_id, Train& train);

  void updateTrain(const Train& train) {
    train_db.update(train.train_id, train);
  }

  sjtu::vector<FixedString<20>> queryStation(const std::string& station_id);

  sjtu::vector<FixedString<20>> queryStation(const FixedString<30>& station_id);

  sjtu::vector<FixedString<20>> queryRoute(const Route& route);
};
