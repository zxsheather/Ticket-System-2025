#pragma once
#include <string>
#include <cstring>

template <size_t N>
struct FixedString {
  char string[N + 1];
  size_t length;

  FixedString() : length(0) {}

  FixedString(const std::string& other) {
    strcpy(string, other.c_str());
    length = other.length();
  }

  std::string toString() const {
    return std::string(string, length);
  }

  int comparedToString(const std::string& other){
    if (length < other.length()) {
      return -1;
    } else if (length > other.length()) {
      return 1;
    } else {
      return strcmp(string, other.c_str());
    }
  }

  bool operator<(const FixedString& other) const {
    return strcmp(string, other.string) < 0;
  }

  bool operator>(const FixedString& other) const {
    return strcmp(string, other.string) > 0;
  }

  bool operator==(const FixedString& other) const {
    return strcmp(string, other.string) == 0;
  }

  bool operator!=(const FixedString& other) const {
    return strcmp(string, other.string) != 0;
  }

  bool operator<=(const FixedString& other) const {
    return strcmp(string, other.string) <= 0;
  }

  bool operator>=(const FixedString& other) const {
    return strcmp(string, other.string) >= 0;
  }

  FixedString& operator=(const std::string& other) {
    strcpy(string, other.c_str());
    length = other.length();
    return *this;
  }

  FixedString& operator=(const FixedString& other) {
    strcpy(string, other.string);
    length = other.length;
    return *this;
  }
};