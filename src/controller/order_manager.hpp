#pragma once
#include "../model/order.hpp"
#include "../model/train.hpp"
#include "../storage/bplus_tree.hpp"
#include "../utilities/limited_sized_string.hpp"

class OrderManager {
 private:
  BPT<FixedString<20>, Order> order_db;  // username -> order
  BPT<UniTrain, Order> pending_db;       // train_id -> pending order
 public:
  OrderManager();
  void addOrder(const Order& order);
  void addPendingOrder(const Order& order);
  void updateOrderStatus(const std::string& username, const Order& order,
                         OrderStatus status);
  sjtu::vector<Order> queryOrder(const std::string& username);
  sjtu::vector<Order> queryPendingOrder(const std::string& train_id,
                                        const Date& date);

  int queryRefundedOrder(const std::string& username, const Order& order);
};
