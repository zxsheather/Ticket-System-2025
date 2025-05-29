#pragma once
#include <cstdint>

#include "../model/order.hpp"
#include "../storage/bplus_tree.hpp"
#include "../utilities/limited_sized_string.hpp"

class OrderManager {
 private:
  BPT<FixedString<20>, Order> order_db;  // username -> order
  BPT<uint64_t, Order> pending_db;       // hashed UniTrain -> pending order
 public:
  OrderManager();
  void addOrder(const Order& order);
  void addPendingOrder(const Order& order);
  void updateOrderStatus(const std::string& username, const Order& order,
                         OrderStatus status);
  void updateOrderStatus(const FixedString<20>& username, const Order& order,
                         OrderStatus status);
  void removeFromPending(const FixedString<20>& train_id, const Date& date,
                         const Order& order);
  sjtu::vector<Order> queryOrder(const std::string& username);
  sjtu::vector<Order> queryPendingOrder(const FixedString<20>& train_id,
                                        const Date& date);
};
