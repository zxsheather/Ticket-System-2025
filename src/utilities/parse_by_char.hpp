#include <string>
#include <sstream>

#include "limited_sized_string.hpp"

template<size_t N>
inline void parse_by_char(const std::string& str, char delimiter, FixedString<N>* result) {
    std::istringstream stream(str);
    std::string token;
    size_t pos = 0;
    while(std::getline(stream, token, delimiter)) {
        result[pos] = token;
        pos++;
    }
}

inline void parse_by_char(const std::string& str, char delimiter, int* result) {
    std::istringstream stream(str);
    std::string token;
    size_t pos = 0;
    while(std::getline(stream, token, delimiter)) {
        result[pos] = std::stoi(token);
        pos++;
    }
}

inline void parse_by_char_accumulate(const std::string& str, char delimiter, int* result) {
    std::istringstream stream(str);
    std::string token;
    size_t pos = 0;
    while(std::getline(stream, token, delimiter)) {
        result[pos] = std::stoi(token) + (pos == 0 ? 0 : result[pos - 1]);
        pos++;
    }
}
        

inline std::string first_word(const std::string& str, char delimiter) {
    std::string result;
    std::istringstream stream(str);
    std::getline(stream, result, delimiter);
    return result;
}