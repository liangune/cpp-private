#include "httpCookie.h"
#include "stringUtils.h"
#include "platform.h"
#include <sstream>
#include <stdio.h>

namespace workflowhttp {

HttpCookie::HttpCookie() {
    Init();
}

void HttpCookie::Init()  {
    maxAge = 0;
    secure = false;
    httponly = false;
    samesite = Default;
    priority = NotSet;
}

void HttpCookie::Reset() {
    Init();
    name.clear();
    value.clear();
    domain.clear();
    path.clear();
    expires.clear();
    kv.clear();
}

bool HttpCookie::Parse(const std::string& str) {
    std::stringstream ss;
    ss << str;
    std::string line;
    std::string::size_type pos;
    std::string key;
    std::string val;

    Reset();
    while (std::getline(ss, line, ';')) {
        pos = line.find_first_of('=');
        if (pos != std::string::npos) {
            key = StringUtils::trim(line.substr(0, pos));
            val = StringUtils::trim(line.substr(pos+1));
            const char* pkey = key.c_str();
            if (stricmp(pkey, "Domain") == 0) {
                domain = val;
            }
            else if (stricmp(pkey, "Path") == 0) {
                path = val;
            }
            else if (stricmp(pkey, "Expires") == 0) {
                expires = val;
            }
            else if (stricmp(pkey, "Max-Age") == 0) {
                maxAge = atoi(val.c_str());
            }
            else if (stricmp(pkey, "SameSite") == 0) {
                samesite =  stricmp(val.c_str(), "Strict") == 0 ? HttpCookie::SameSite::Strict :
                            stricmp(val.c_str(), "Lax")    == 0 ? HttpCookie::SameSite::Lax    :
                            stricmp(val.c_str(), "None")   == 0 ? HttpCookie::SameSite::None   :
                                                                  HttpCookie::SameSite::Default;
            }
            else if (stricmp(pkey, "Priority") == 0) {
                priority =  stricmp(val.c_str(), "Low")    == 0 ? HttpCookie::Priority::Low    :
                            stricmp(val.c_str(), "Medium") == 0 ? HttpCookie::Priority::Medium :
                            stricmp(val.c_str(), "High")   == 0 ? HttpCookie::Priority::High   :
                                                                  HttpCookie::Priority::NotSet ;
            }
            else {
                if (name.empty()) {
                    name = key;
                    value = val;
                }
                kv[key] = val;
            }
        } else {
            key = StringUtils::trim(line);
            const char* pkey = key.c_str();
            if (stricmp(pkey, "Secure") == 0) {
                secure = true;
            }
            else if (stricmp(pkey, "HttpOnly") == 0) {
                httponly = true;
            }
            else {
                //printf("Unrecognized key '%s'", key.c_str());
            }
        }

    }
    return !name.empty();
}

std::string HttpCookie::String() const {
    //assert(!name.empty() || !kv.empty());
    if (!name.empty() || !kv.empty()) {
        return "";
    }
    std::string res;

    if (!name.empty()) {
        res = name;
        res += "=";
        res += value;
    }

    for (auto& pair : kv) {
        if (pair.first == name) continue;
        if (!res.empty()) res += "; ";
        res += pair.first;
        res += "=";
        res += pair.second;
    }

    if (!domain.empty()) {
        res += "; Domain=";
        res += domain;
    }

    if (!path.empty()) {
        res += "; Path=";
        res += path;
    }

    if (maxAge > 0) {
        res += "; Max-Age=";
        res += std::to_string(maxAge);
    } else if (!expires.empty()) {
        res += "; Expires=";
        res += expires;
    }

    if (samesite != HttpCookie::SameSite::Default) {
        res += "; SameSite=";
        res += samesite == HttpCookie::SameSite::Strict ? "Strict" :
               samesite == HttpCookie::SameSite::Lax    ? "Lax"    :
                                                          "None"   ;
    }

    if (priority != HttpCookie::Priority::NotSet) {
        res += "; Priority=";
        res += priority == HttpCookie::Priority::Low    ? "Low"    :
               priority == HttpCookie::Priority::Medium ? "Medium" :
                                                          "High"   ;
    }

    if (secure) {
        res += "; Secure";
    }

    if (httponly) {
        res += "; HttpOnly";
    }

    return res;
}

}