#pragma once
#include <string>

template <size_t N>
struct FixedString {
  char string[N + 1];
  size_t length;

  FixedString(const std::string& other) {
    strcpy(string, other.c_str());
    length = other.length();
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