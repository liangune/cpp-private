#include "url.h"
#include <string.h>

#ifndef IS_HEX
#define IS_HEX(c) (IS_DIGIT(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#endif

#ifndef IS_ALPHA
#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#endif

#ifndef IS_DIGIT
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#endif

#ifndef IS_ALPHANUM
#define IS_ALPHANUM(c) (IS_ALPHA(c) || IS_DIGIT(c))
#endif

static inline char* strnchr(const char* s, char c, size_t n) {
    if (s == NULL) {
        return NULL;
    }
    const char* p = s;
    while (*p != '\0' && n-- > 0) {
        if (*p == c) return (char*)p;
        ++p;
    }
    return NULL;
}

static inline int parse_url(url_t* stURL, const char* strURL) {
    if (stURL == NULL || strURL == NULL) return -1;
    memset(stURL, 0, sizeof(url_t));
    const char* begin = strURL;
    const char* end = strURL;
    while (*end != '\0') ++end;
    if (end - begin > 65535) return -2;
    // scheme://
    const char* sp = strURL;
    const char* ep = strstr(sp, "://");
    if (ep) {
        // stURL->fields[URL_SCHEME].off = sp - begin;
        stURL->fields[URL_SCHEME].len = ep - sp;
        sp = ep + 3;
    }
    // user:pswd@host:port
    ep = strchr(sp, '/');
    if (ep == NULL) ep = end;
    const char* user = sp;
    const char* host = sp;
    const char* pos = strnchr(sp, '@', ep - sp);
    if (pos) {
        // user:pswd
        const char* pswd = strnchr(user, ':', pos - user);
        if (pswd) {
            stURL->fields[URL_PASSWORD].off = pswd + 1 - begin;
            stURL->fields[URL_PASSWORD].len = pos - pswd - 1;
        } else {
            pswd = pos;
        }
        stURL->fields[URL_USERNAME].off = user - begin;
        stURL->fields[URL_USERNAME].len = pswd - user;
        // @
        host = pos + 1;
    }
    // port
    const char* port = strnchr(host, ':', ep - host);
    if (port) {
        stURL->fields[URL_PORT].off = port + 1 - begin;
        stURL->fields[URL_PORT].len = ep - port - 1;
        // atoi
        for (unsigned short i = 1; i <= stURL->fields[URL_PORT].len; ++i) {
            stURL->port = stURL->port * 10 + (port[i] - '0');
        }
    } else {
        port = ep;
        // set default port
        stURL->port = 80;
        if (stURL->fields[URL_SCHEME].len > 0) {
            if (strncmp(strURL, "https://", 8) == 0) {
                stURL->port = 443;
            }
        }
    }
    // host
    stURL->fields[URL_HOST].off = host - begin;
    stURL->fields[URL_HOST].len = port - host;
    if (ep == end) return 0;
    // /path
    sp = ep;
    ep = strchr(sp, '?');
    if (ep == NULL) ep = end;
    stURL->fields[URL_PATH].off = sp - begin;
    stURL->fields[URL_PATH].len = ep - sp;
    if (ep == end) return 0;
    // ?query
    sp = ep + 1;
    ep = strchr(sp, '#');
    if (ep == NULL) ep = end;
    stURL->fields[URL_QUERY].off = sp - begin;
    stURL->fields[URL_QUERY].len = ep - sp;
    if (ep == end) return 0;
    // #fragment
    sp = ep + 1;
    ep = end;
    stURL->fields[URL_FRAGMENT].off = sp - begin;
    stURL->fields[URL_FRAGMENT].len = ep - sp;
    return 0;
}

static inline bool is_unambiguous(char c) {
    return IS_ALPHANUM(c) ||
           c == '-' ||
           c == '_' ||
           c == '.' ||
           c == '~';
}

static inline bool char_in_str(char c, const char* str) {
    const char* p = str;
    while (*p && *p != c) ++p;
    return *p != '\0';
}

static inline unsigned char hex2i(char hex) {
    return hex <= '9' ? hex - '0' :
        hex <= 'F' ? hex - 'A' + 10 : hex - 'a' + 10;
}

std::string CUrl::escape(const std::string& str, const char* unescaped_chars) {
    std::string ostr;
    static char tab[] = "0123456789ABCDEF";
    const unsigned char* p = reinterpret_cast<const unsigned char*>(str.c_str());
    char szHex[4] = "%00";
    while (*p != '\0') {
        if (is_unambiguous(*p) || char_in_str(*p, unescaped_chars)) {
            ostr += *p;
        }
        else {
            szHex[1] = tab[*p >> 4];
            szHex[2] = tab[*p & 0xF];
            ostr += szHex;
        }
        ++p;
    }
    return ostr;
}

std::string CUrl::unescape(const std::string& str) {
    std::string ostr;
    const char* p = str.c_str();
    while (*p != '\0') {
        if (*p == '%' &&
            IS_HEX(p[1]) &&
            IS_HEX(p[2])) {
            ostr += ((hex2i(p[1]) << 4) | hex2i(p[2]));
            p += 3;
        }
        else {
            if (*p == '+') {
                ostr += ' ';
            } else {
                ostr += *p;
            }
            ++p;
        }
    }
    return ostr;
}

void CUrl::reset() {
    url.clear();
    scheme.clear();
    username.clear();
    password.clear();
    host.clear();
    port = 0;
    path.clear();
    query.clear();
    fragment.clear();
}

bool CUrl::parse(const std::string& url) {
    reset();
    this->url = url;
    url_t stURL;
    if (parse_url(&stURL, url.c_str()) != 0) {
        return false;
    }
    int len = stURL.fields[URL_SCHEME].len;
    if (len > 0) {
        scheme = url.substr(stURL.fields[URL_SCHEME].off, len);
    }
    len = stURL.fields[URL_USERNAME].len;
    if (len > 0) {
        username = url.substr(stURL.fields[URL_USERNAME].off, len);
        len = stURL.fields[URL_PASSWORD].len;
        if (len > 0) {
            password = url.substr(stURL.fields[URL_PASSWORD].off, len);
        }
    }
    len = stURL.fields[URL_HOST].len;
    if (len > 0) {
        host = url.substr(stURL.fields[URL_HOST].off, len);
    }
    port = stURL.port;
    len = stURL.fields[URL_PATH].len;
    if (len > 0) {
        path = url.substr(stURL.fields[URL_PATH].off, len);
    } else {
        path = "/";
    }
    len = stURL.fields[URL_QUERY].len;
    if (len > 0) {
        query = url.substr(stURL.fields[URL_QUERY].off, len);
    }
    len = stURL.fields[URL_FRAGMENT].len;
    if (len > 0) {
        fragment = url.substr(stURL.fields[URL_FRAGMENT].off, len);
    }
    return true;
}

const std::string& CUrl::dump() {
    url.clear();
    // scheme://
    if (!scheme.empty()) {
        url += scheme;
        url += "://";
    }
    // user:pswd@
    if (!username.empty()) {
        url += username;
        if (!password.empty()) {
            url += ":";
            url += password;
        }
        url += "@";
    }
    // host:port
    if (!host.empty()) {
        url += host;
        if (port != 80 && port != 443) {
            char buf[16] = {0};
            snprintf(buf, sizeof(buf), ":%d", port);
            url += port;
        }
    }
    // /path
    if (!path.empty()) {
        url += path;
    }
    // ?query
    if (!query.empty()) {
        url += '?';
        url += query;
    }
    // #fragment
    if (!fragment.empty()) {
        url += '#';
        url += fragment;
    }
    return url;
}
