#include "command_system.hpp"

#include <cstring>
#include <iostream>

#include "../utilities/hash.hpp"

ParamMap::ParamMap() {
  std::memset(exists, 0, sizeof(exists));
}

bool ParamMap::has(char key) const {
  return exists[key - 'a'];
}

std::string ParamMap::get(char key) const {
  return values[key - 'a'];
}

void ParamMap::set(char key, const std::string& value) {
  values[key - 'a'] = value;
  exists[key - 'a'] = true;
}

void ParamMap::clear() {
  memset(exists, 0, sizeof(exists));
}

CommandSystem::CommandSystem() {
  memset(handlers, 0, sizeof(handlers));
}

CommandSystem::~CommandSystem() {
  for (int i = 0; i < handler_count; ++i) {
    delete handlers[i];
  }
}

void CommandSystem::registerHandler(const std::string& cmd_name,
                                    CommandHandler* handler) {
  positions[cmd_name] = handler_count;
  handlers[handler_count] = handler;
  handler_count++;
}

std::string CommandSystem::parseAndExecute(const std::string& cmd_line,
                                           std::string& timestamp) {
  std::string cmd_name;
  ParamMap params;

  size_t pos = 1;
  while (cmd_line[pos] != ']') {
    timestamp += cmd_line[pos];
    pos++;
  }

  pos += 2;
  while (pos < cmd_line.size() && cmd_line[pos] != ' ') {
    cmd_name += cmd_line[pos];
    pos++;
  }

  pos++;
  while (pos < cmd_line.size() && cmd_line[pos] == '-') {
    char key = cmd_line[pos + 1];
    pos += 3;
    std::string value;
    while (pos < cmd_line.size() && cmd_line[pos] != ' ') {
      value += cmd_line[pos];
      pos++;
    }
    params.set(key, value);
    pos++;
  }
  if (positions.find(cmd_name) == positions.end()) {
    std::cerr << "Command not found: " << cmd_name << std::endl;
    return "bye";
  }
  size_t idx = positions[cmd_name];
  std::string result = handlers[idx]->execute(params, timestamp);
  return result;
}
