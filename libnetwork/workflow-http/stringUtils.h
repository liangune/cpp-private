#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include <string>
#include <vector>

#include <iostream>
#include <sstream>

#include "platform.h"
#include "typedef.h"

#define SPACE_CHARS     " \t\r\n"
#define PAIR_CHARS      "{}[]()<>\"\"\'\'``"

namespace workflowhttp {

typedef std::vector<std::string> StringList;

// std::map<std::string, std::string, StringCaseLess>
class StringCaseLess : public std::less<std::string> {
public:
    bool operator()(const std::string& lhs, const std::string& rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

class StringUtils {
public:
    // NOTE: low-version NDK not provide std::to_string
    template<typename T>
    static std::string to_string(const T& t) {
        std::ostringstream oss;
        oss << t;
        return oss.str();
    }

    template<typename T>
    static T from_string(const std::string& str) {
        T t;
        std::istringstream iss(str);
        iss >> t;
        return t;
    }

    static std::string& toupper(std::string& str);
    static std::string& tolower(std::string& str);
    static std::string& reverse(std::string& str);

    static bool startswith(const std::string& str, const std::string& start);
    static bool endswith(const std::string& str, const std::string& end);
    static bool contains(const std::string& str, const std::string& sub);

    static std::string asprintf(const char* fmt, ...);
    // x,y,z
    static StringList split(const std::string& str, char delim = ',');
    // k1=v1&k2=v2
    static KeyValue splitKV(const std::string& str, char kv_kv = '&', char k_v = '=');
    static std::string trim(const std::string& str, const char* chars = SPACE_CHARS);
    static std::string ltrim(const std::string& str, const char* chars = SPACE_CHARS);
    static std::string rtrim(const std::string& str, const char* chars = SPACE_CHARS);
    static std::string trim_pairs(const std::string& str, const char* pairs = PAIR_CHARS);std::string replace(const std::string& str, const std::string& find, const std::string& rep);
    static std::string replaceAll(const std::string& str, const std::string& find, const std::string& rep);
};

}

#endif // !_STRING_UTILS_H_
