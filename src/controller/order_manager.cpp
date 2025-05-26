#include "order_manager.hpp"

OrderManager::OrderManager() : order_db("order"), pending_db("pending") {}

void OrderManager::addOrder(const Order& order) {
  order_db.insert(order.username, order);
}

void OrderManager::addPendingOrder(const Order& order) {
  pending_db.insert(UniTrain(order.train_id, order.origin_station_date), order);
}

sjtu::vector<Order> OrderManager::queryOrder(const std::string& username) {
  return order_db.find(username);
}

void OrderManager::updateOrderStatus(const std::string& username,
                                     const Order& order, OrderStatus status) {
  if (status == order.status) {
    return;
  }
  Order updated_order = order;
  updated_order.status = status;
  order_db.update(username, updated_order, order);
}
void OrderManager::updateOrderStatus(const FixedString<20>& username,
                                     const Order& order, OrderStatus status) {
  if (status == order.status) {
    return;
  }
  Order updated_order = order;
  updated_order.status = status;
  order_db.update(username, updated_order, order);
}

void OrderManager::removeFromPending(const UniTrain& unitrain,
                                     const Order& order) {
  pending_db.remove(unitrain, order);
}

sjtu::vector<Order> OrderManager::queryPendingOrder(const UniTrain& unitrain) {
  return pending_db.find(unitrain);
}