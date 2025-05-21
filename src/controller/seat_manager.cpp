#include "seat_manager.hpp"

SeatManager::SeatManager(): seat_db("seat"){}

SeatMap SeatManager::querySeat(const UniTrain& unitrain){
    return seat_db.find(unitrain)[0];
}