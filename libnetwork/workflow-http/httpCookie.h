#ifndef _HTTP_COOKIE_H_
#define _HTTP_COOKIE_H_

#include <string>
#include <map>
#include "typedef.h"

namespace workflowhttp {

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Set-Cookie
// Cookie: sessionid=1; domain=.example.com; path=/; max-age=86400; secure; httponly
class HttpCookie {
public:
    HttpCookie();

    void Init();
    void Reset();

    bool Parse(const std::string& str);
    std::string String() const;

private:
    std::string name;
    std::string value;
    std::string domain;
    std::string path;
    std::string expires;
    int         maxAge;
    bool        secure;
    bool        httponly;
    enum SameSite {
        Default,
        Strict,
        Lax,
        None
    } samesite;
    enum Priority {
        NotSet,
        Low,
        Medium,
        High,
    } priority;
    KeyValue kv; // for multiple names
};

}

#endif // !_HTTP_COOKIE_H_