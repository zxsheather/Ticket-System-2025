
#include "../model/train.hpp"
#include "../storage/bplus_tree.hpp"

class TrainManager{
 private:
  BPT<long long, Train> train_db;
  BPT<long long, FixedString<20>> station_db;
 public:
  TrainManager();
  
  int addTrain(const Train& train);

  int deleteTrain(const std::string& train_id);

  int releaseTrain(const std::string& train_id);

  int queryTrain(const std::string& train_id, Train& train);
};
  