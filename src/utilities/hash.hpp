#pragma once
#include <string>

class Hash {
 private:
  static const long long PR = 998244353;
  static const long long MOD = 99234523452349217;

 public:
  static int hashCommand(std::string cmd, const int mod) {
    int hash = 0;
    for (char c : cmd) {
      hash = (hash * 31 + c) % mod;
    }
    return hash;
  }

  static long long hashKey(std::string s) {
    __int128_t hash = 0;
    for (char c : s) {
      hash = (hash * PR + c + 1) % MOD;
    }
    return hash;
  }
};