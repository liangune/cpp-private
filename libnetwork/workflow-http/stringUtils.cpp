#include "stringUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

namespace workflowhttp {

std::string& StringUtils::toupper(std::string& str) {
    // std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    char* p = (char*)str.c_str();
    while (*p != '\0') {
        if (*p >= 'a' && *p <= 'z') {
            *p &= ~0x20;
        }
        ++p;
    }
    return str;
}

std::string& StringUtils::tolower(std::string& str) {
    // std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    char* p = (char*)str.c_str();
    while (*p != '\0') {
        if (*p >= 'A' && *p <= 'Z') {
            *p |= 0x20;
        }
        ++p;
    }
    return str;
}

std::string& StringUtils::reverse(std::string& str) {
    // std::reverse(str.begin(), str.end());
    char* b = (char*)str.c_str();
    char* e = b + str.length() - 1;
    char tmp;
    while (e > b) {
        tmp = *e;
        *e = *b;
        *b = tmp;
        --e;
        ++b;
    }
    return str;
}

bool StringUtils::startswith(const std::string& str, const std::string& start) {
    if (str.length() < start.length()) return false;
    return str.compare(0, start.length(), start) == 0;
}

bool StringUtils::endswith(const std::string& str, const std::string& end) {
    if (str.length() < end.length()) return false;
    return str.compare(str.length() - end.length(), end.length(), end) == 0;
}

bool StringUtils::contains(const std::string& str, const std::string& sub) {
    if (str.length() < sub.length()) return false;
    return str.find(sub) != std::string::npos;
}

static inline int vscprintf(const char* fmt, va_list ap) {
    return vsnprintf(NULL, 0, fmt, ap);
}

std::string StringUtils::asprintf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int len = vscprintf(fmt, ap);
    va_end(ap);

    std::string str;
    str.reserve(len+1);
    // must resize to set str.size
    str.resize(len);
    // must recall va_start on unix
    va_start(ap, fmt);
    vsnprintf((char*)str.data(), len+1, fmt, ap);
    va_end(ap);

    return str;
}

StringList StringUtils::split(const std::string& str, char delim) {
    /*
    std::stringstream ss;
    ss << str;
    string item;
    StringList res;
    while (std::getline(ss, item, delim)) {
        res.push_back(item);
    }
    return res;
    */
    const char* p = str.c_str();
    const char* value = p;
    StringList res;
    while (*p != '\0') {
        if (*p == delim) {
            res.push_back(std::string(value, p-value));
            value = p+1;
        }
        ++p;
    }
    res.push_back(value);
    return res;
}

KeyValue StringUtils::splitKV(const std::string& str, char kv_kv, char k_v) {
    enum {
        s_key,
        s_value,
    } state = s_key;
    const char* p = str.c_str();
    const char* key = p;
    const char* value = NULL;
    int key_len = 0;
    int value_len = 0;
    KeyValue kvs;
    while (*p != '\0') {
        if (*p == kv_kv) {
            if (key_len && value_len) {
                kvs[std::string(key, key_len)] = std::string(value, value_len);
                key_len = value_len = 0;
            }
            state = s_key;
            key = p+1;
        }
        else if (*p == k_v) {
            state = s_value;
            value = p+1;
        }
        else {
            state == s_key ? ++key_len : ++value_len;
        }
        ++p;
    }
    if (key_len && value_len) {
        kvs[std::string(key, key_len)] = std::string(value, value_len);
    }
    return kvs;
}

std::string StringUtils::trim(const std::string& str, const char* chars) {
    std::string::size_type pos1 = str.find_first_not_of(chars);
    if (pos1 == std::string::npos)   return "";

    std::string::size_type pos2 = str.find_last_not_of(chars);
    return str.substr(pos1, pos2-pos1+1);
}

std::string StringUtils::ltrim(const std::string& str, const char* chars) {
    std::string::size_type pos = str.find_first_not_of(chars);
    if (pos == std::string::npos)    return "";
    return str.substr(pos);
}

std::string StringUtils::rtrim(const std::string& str, const char* chars) {
    std::string::size_type pos = str.find_last_not_of(chars);
    return str.substr(0, pos+1);
}

std::string StringUtils::trim_pairs(const std::string& str, const char* pairs) {
    const char* s = str.c_str();
    const char* e = str.c_str() + str.size() - 1;
    const char* p = pairs;
    bool is_pair = false;
    while (*p != '\0' && *(p+1) != '\0') {
        if (*s == *p && *e == *(p+1)) {
            is_pair = true;
            break;
        }
        p += 2;
    }
    return is_pair ? str.substr(1, str.size()-2) : str;
}

std::string StringUtils::replace(const std::string& str, const std::string& find, const std::string& rep) {
    std::string res(str);
    std::string::size_type pos = res.find(find);
    if (pos != std::string::npos) {
        res.replace(pos, find.size(), rep);
    }
    return res;
}

std::string StringUtils::replaceAll(const std::string& str, const std::string& find, const std::string& rep) {
    std::string::size_type pos = 0;
    std::string::size_type a = find.size();
    std::string::size_type b = rep.size();

    std::string res(str);
    while ((pos = res.find(find, pos)) != std::string::npos) {
        res.replace(pos, a, rep);
        pos += b;
    }
    return res;
}

} // end namespace 
