#ifndef _HTTP_URL_PARSER_H_
#define _HTTP_URL_PARSER_H_

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>

namespace workflowhttp {
class URLParser {
public:
    std::string schema;
    std::string host;
    int port;
    std::string path;
    std::string query;

    URLParser() = default;
    URLParser(const std::string& url);
    int Parse(const std::string& url);
};
}

#endif 