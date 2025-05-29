#pragma once
#include <cstdint>  // Include for uint32_t and uint64_t
#include <string>

#include "../model/time.hpp"
#include "limited_sized_string.hpp"

class Hash {
 public:
  static int hashCommand(const std::string& cmd, const int mod) {
    uint32_t hash = 5381;  // djb2哈希起始值
    for (char c : cmd) {
      hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    return hash % mod;
  }

  template <int N>
  static uint64_t hashKey(const FixedString<N>& s) {
    // 使用内存块哈希方法 - 速度极快
    const char* data = s.string;
    size_t len = s.length;

    uint64_t hash = 0xCBF29CE484222325ULL;  // FNV偏移基数

    // 每次处理8个字节
    size_t i = 0;
    for (; i + 8 <= len; i += 8) {
      uint64_t chunk;
      memcpy(&chunk, data + i, 8);  // 避免对齐问题
      hash ^= chunk;
      hash *= 0x100000001B3ULL;  // FNV质数
    }

    // 处理剩余字节
    for (; i < len; ++i) {
      hash ^= data[i];
      hash *= 0x100000001B3ULL;
    }

    return hash;
  }

  static uint64_t hashKey(const std::string& s) {
    // 字符串哈希 - 速度优化版本
    const char* data = s.data();
    size_t len = s.length();

    if (len <= 16) {  // 短字符串快速路径
      uint64_t hash = 0;
      for (size_t i = 0; i < len; ++i) {
        hash = hash * 33 + data[i];
      }
      return hash;
    }

    // 长字符串采用8字节块处理
    uint64_t hash = 0;
    size_t i = 0;
    for (; i + 8 <= len; i += 8) {
      uint64_t chunk;
      memcpy(&chunk, data + i, 8);
      hash ^= chunk;
      hash = (hash << 13) | (hash >> 51);  // 循环移位
    }

    // 处理剩余字节
    for (; i < len; ++i) {
      hash = hash * 33 + data[i];
    }

    return hash;
  }

  static uint64_t hashKey(const FixedString<20>& s, const Date& date) {
    // 组合哈希 - 无需模运算
    uint64_t hash = hashKey<20>(s);
    uint64_t dateHash = (date.month << 5) | date.day;

    // 应用混合函数
    dateHash = ((dateHash << 16) ^ (dateHash >> 16)) * 0x45d9f3b;
    dateHash = ((dateHash << 16) ^ (dateHash >> 16)) * 0x45d9f3b;

    return hash ^ dateHash;
  }
};