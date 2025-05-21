#include "train_manager.hpp"
#include "../utilities/hash.hpp"

TrainManager::TrainManager() : train_db("train"), station_db("station"){}
int TrainManager::addTrain(const Train& train) {
    long long hash = Hash::hashKey(train.train_id.toString());
    if(train_db.exists(hash)){
        return -1;
    }
    train_db.insert(hash, train);
    return 0;
}

int TrainManager::deleteTrain(const std::string& train_id) {
    long long hash = Hash::hashKey(train_id);
    sjtu::vector<Train> result = train_db.find(hash);
    if (result.empty() || result[0].is_released) {
        return -1;
    }
    train_db.remove(hash, result[0]);
    return 0;
}

int TrainManager::releaseTrain(const std::string& train_id) {
    long long hash = Hash::hashKey(train_id);
    sjtu::vector<Train> result = train_db.find(hash);
    if (result.empty() || result[0].is_released) {
        return -1;
    }
    Train& train = result[0];
    train.is_released = true;
    train_db.update(hash, train);
    for(auto& station : train.stations){
        station_db.insert(Hash::hashKey(station.toString()), train_id);
    }
    return 0;
}

int TrainManager::queryTrain(const std::string& train_id, Train& train) {
    long long hash = Hash::hashKey(train_id);
    sjtu::vector<Train> result = train_db.find(hash);
    if (result.empty()) {
        return -1;
    }
    train = result[0];
    return 0;
}