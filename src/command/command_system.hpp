#pragma once

#include <string>

#include "../stl/map.hpp"

class ParamMap {
 private:
  static const int MAX_PARAMS = 25;
  std::string values[MAX_PARAMS];
  bool exists[MAX_PARAMS];

 public:
  ParamMap();
  bool has(char key) const;
  std::string get(char key) const;
  void set(char key, const std::string& value);
  void clear();
};

class CommandHandler {
 public:
  virtual void execute(const ParamMap& params,
                       const std::string& timestamp) = 0;
  virtual ~CommandHandler() {}
};

class CommandSystem {
 private:
  sjtu::map<std::string, size_t> positions;
  CommandHandler* handlers[30];
  size_t handler_count{0};

 public:
  CommandSystem();
  ~CommandSystem();

  void registerHandler(const std::string& cmd_name, CommandHandler* handler);

  void parseAndExecute(const std::string& cmd_line, std::string& timestamp,
                       std::string& cmd_name);
};
