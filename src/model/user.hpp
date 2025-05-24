#pragma once
#include "../utilities/limited_sized_string.hpp"

struct User {
  FixedString<20> username{};
  FixedString<30> password{};
  FixedString<20> name{};
  FixedString<30> mail_addr{};
  int privilege{};

  User() {}

  User(const std::string& username, const std::string& password,
       const std::string& name, const std::string& mail_addr,
       const int& privilege)
      : username(username),
        password(password),
        name(name),
        mail_addr(mail_addr),
        privilege(privilege) {}

  bool operator<(const User& other) const { return username < other.username; }
  bool operator>(const User& other) const { return username > other.username; }
  bool operator==(const User& other) const {
    return username == other.username;
  }
  bool operator!=(const User& other) const {
    return username != other.username;
  }
  bool operator<=(const User& other) const {
    return username <= other.username;
  }
  bool operator>=(const User& other) const {
    return username >= other.username;
  }
};

struct UserProfile {
  std::string username;
  std::string name;
  std::string mail_addr;
  int privilege;
};