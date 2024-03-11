#ifndef _URL_H_
#define _URL_H_

#include <string> // import std::string

// scheme:[//[user[:password]@]host[:port]][/path][?query][#fragment]
typedef enum {
    URL_SCHEME,
    URL_USERNAME,
    URL_PASSWORD,
    URL_HOST,
    URL_PORT,
    URL_PATH,
    URL_QUERY,
    URL_FRAGMENT,
    URL_FIELD_NUM,
} url_field_e;

typedef struct url_s {
    struct {
        unsigned short off;
        unsigned short len;
    } fields[URL_FIELD_NUM];
    unsigned short port;
} url_t;


class CUrl {
public:
    static std::string escape(const std::string& str, const char* unescaped_chars = "");
    static std::string unescape(const std::string& str);
    static inline std::string escapeUrl(const std::string& url) {
        return escape(url, ":/@?=&#+");
    }

    CUrl() : port(0) {}
    ~CUrl() {}

    void reset();
    bool parse(const std::string& url);
    const std::string& dump();

    std::string url;
    std::string scheme;
    std::string username;
    std::string password;
    std::string host;
    int         port;
    std::string path;
    std::string query;
    std::string fragment;
};

#endif // _URL_H_
