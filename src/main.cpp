// The entrance for the program

#include <iostream>
#include "command/command_system.hpp"

int main(){
    std::string line;
    CommandSystem command_system;
    while (getline(std::cin, line)) {
        std::string timestamp;
        std::string result = command_system.parseAndExecute(line, timestamp);
        std::cout << '[' + timestamp + "] " + result << '\n';
        if(result == "bye"){
            break;
        }
    }
    return 0;
}