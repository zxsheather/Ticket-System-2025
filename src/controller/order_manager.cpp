#include "order_manager.hpp"

#include "../utilities/hash.hpp"

OrderManager::OrderManager() : order_db("order"), pending_db("pending") {}

void OrderManager::addOrder(const Order& order) {
  order_db.insert(order.username, order);
}

void OrderManager::addPendingOrder(const Order& order) {
  long long hashed_key =
      Hash::hashKey(order.train_id, order.origin_station_date);
  pending_db.insert(hashed_key, order);
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

void OrderManager::removeFromPending(const FixedString<20>& unitrain,
                                     const Date& date, const Order& order) {
  long long hashed_key = Hash::hashKey(unitrain, date);
  pending_db.remove(hashed_key, order);
}

sjtu::vector<Order> OrderManager::queryPendingOrder(
    const FixedString<20>& train_id, const Date& date) {
  long long unitrain = Hash::hashKey(train_id, date);
  return pending_db.find(unitrain);
}