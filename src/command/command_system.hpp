#pragma once

#include <string>

class ParamMap {
 private:
  static const int MAX_PARAMS = 26;
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
  virtual std::string execute(const ParamMap& params) = 0;
  virtual ~CommandHandler() {}
};

class CommandSystem {
 private:
  static const int MAX_HANDLERS = 128;
  CommandHandler* handlers[MAX_HANDLERS];

  int hashCommand(const std::string& cmd) const;

 public:
  CommandSystem();
  ~CommandSystem();

  void registerHandler(const std::string& cmd_name, CommandHandler* handler);

  std::string parseAndExecute(const std::string& cmd_line, std::string& timestamp);
};
