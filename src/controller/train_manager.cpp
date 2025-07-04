#include "train_manager.hpp"

TrainManager::TrainManager()
    : train_db("train"), station_db("station"), route_db("route") {}
int TrainManager::addTrain(const Train& train) {
  if (train_db.exists(train.train_id)) {
    return -1;
  }
  train_db.insert(train.train_id, train);
  return 0;
}

int TrainManager::deleteTrain(const std::string& train_id) {
  sjtu::vector<Train> result = train_db.find(train_id);
  if (result.empty() || result[0].is_released) {
    return -1;
  }
  train_db.remove(train_id, result[0]);
  return 0;
}

int TrainManager::releaseTrain(const std::string& train_id, Train& train) {
  sjtu::vector<Train> result = train_db.find(train_id);
  if (result.empty() || result[0].is_released) {
    return -1;
  }
  train = std::move(result[0]);
  for (size_t i = 0; i < train.station_num; ++i) {
    station_db.insert(train.stations[i], train.train_id);
  }
  for (size_t i = 0; i < train.station_num - 1; ++i) {
    for (size_t j = i + 1; j < train.station_num; ++j) {
      Route route{train.stations[i], train.stations[j]};
      route_db.insert(route, train.train_id);
    }
  }
  return 0;
}

int TrainManager::queryTrain(const std::string& train_id, Train& train) {
  sjtu::vector<Train> result = train_db.find(train_id);
  if (result.empty()) {
    return -1;
  }
  train = result[0];
  return 0;
}

int TrainManager::queryTrain(const FixedString<20>& train_id, Train& train) {
  sjtu::vector<Train> result = train_db.find(train_id);
  if (result.empty()) {
    return -1;
  }
  train = std::move(result[0]);
  return 0;
}

sjtu::vector<FixedString<20>> TrainManager::queryStation(
    const std::string& station_id) {
  return station_db.find(station_id);
}

sjtu::vector<FixedString<20>> TrainManager::queryStation(
    const FixedString<30>& station_id) {
  return station_db.find(station_id);
}

sjtu::vector<FixedString<20>> TrainManager::queryRoute(const Route& route) {
  return route_db.find(route);
}