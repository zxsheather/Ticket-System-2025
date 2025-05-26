#include "order_manager.hpp"

OrderManager::OrderManager() : order_db("order"), pending_db("pending") {}

void OrderManager::addOrder(const Order& order) {
  order_db.insert(order.username, order);
}

void OrderManager::addPendingOrder(const Order& order) {
  pending_db.insert(UniTrain(order.train_id, order.start_time.date), order);
}

sjtu::vector<Order> OrderManager::queryOrder(const std::string& username) {
  return order_db.find(username);
}
