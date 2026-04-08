#include "router.h"

namespace workflowhttp {

void Router::Handle(const std::string &httpMethod, const std::string &relativePath, HandlerFunc handler) {
    AddRoute(httpMethod, relativePath, handler);
}

void Router::CombineHandlers(HandlersChain &handlers, HandlersChain &mergedHandlers) {
    //size_t finalSize = handlers.size() + this->handlers.size();

    HandlersChain::iterator it = this->handlers.begin();
    for(; it != this->handlers.end(); it++) {
        mergedHandlers.push_back(*it);
    }

    it = handlers.begin();
    for(; it != handlers.end(); it++) {
        mergedHandlers.push_back(*it);
    }
}

void Router::AddRoute(const std::string &httpMethod, const std::string &path, HandlerFunc handler) {
    HTTPMethodHandlerFuncMap::iterator it = methodHandlerFuncMap.find(httpMethod);
    auto node = std::shared_ptr<RouterNode>(new RouterNode(path, handler));
    if (it == methodHandlerFuncMap.end()) {
        HTTPHandlerFuncVec handlerVec;
        handlerVec.emplace_back(std::move(node));
        methodHandlerFuncMap[httpMethod] = handlerVec;
    } else {
        it->second.emplace_back(std::move(node));
    }
}

void Router::GET(const std::string& relativePath , HandlerFunc handler) {
    Handle(MethodGet, relativePath, handler);
}

void Router::POST(const std::string& relativePath , HandlerFunc handler) {
    Handle(MethodPost, relativePath, handler);
}

void Router::Delete(const std::string& relativePath , HandlerFunc handler) {
    Handle(MethodDelete, relativePath, handler);
}

void Router::PATCH(const std::string& relativePath , HandlerFunc handler) {
    Handle(MethodPatch, relativePath, handler);
}

void Router::PUT(const std::string& relativePath , HandlerFunc handler) {
    Handle(MethodPut, relativePath, handler);
}

void Router::OPTIONS(const std::string& relativePath , HandlerFunc handler) {
    Handle(MethodOptions, relativePath, handler);
}

void Router::HEAD(const std::string& relativePath , HandlerFunc handler) {
    Handle(MethodHead, relativePath, handler);
}

void Router::Any(const std::string& relativePath, HandlerFunc handler) {
	Handle(MethodGet, relativePath, handler);
	Handle(MethodPost, relativePath, handler);
	Handle(MethodPut, relativePath, handler);
	Handle(MethodPatch, relativePath, handler);
	Handle(MethodHead, relativePath, handler);
	Handle(MethodOptions, relativePath, handler);
	Handle(MethodDelete, relativePath, handler);
	Handle(MethodConnect, relativePath, handler);
	Handle(MethodTrace, relativePath, handler);
}

bool Router::Strendswith(const char* str, const char* end) {
    if (str == NULL || end == NULL) {
        return false;
    }

    int len1 = 0;
    int len2 = 0;
    while (*str) {++str; ++len1;}
    while (*end) {++end; ++len2;}
    if (len1 < len2) return false;
    while (len2-- > 0) {
        --str;
        --end;
        if (*str != *end) {
            return false;
        }
    }
    return true;
}

RouterNode* Router::GetRouterNode(const std::string& path, HTTPHandlerFuncVec &handers, Params &params) {
    // {baseUrl}/path?query
    const char* s = path.c_str();
    const char* b = baseUrl.c_str();
    while (*s && *b && *s == *b) {++s;++b;}
    if (*b != '\0') {
        return nullptr;
    }
    const char* e = s;
    while (*e && *e != '?') ++e;

    std::string pathTmp = std::string(s, e);
    const char *kp, *ks, *vp, *vs;
    bool match;
    for (auto iter = handers.begin(); iter != handers.end(); ++iter) {
        kp = iter->get()->fullPath.c_str();
        vp = pathTmp.c_str();
        match = false;
        Params vecParams;

        while (*kp && *vp) {
            if (kp[0] == '*') {
                // wildcard *
                match = Strendswith(vp, kp+1);
                break;
            } else if (*kp != *vp) {
                match = false;
                break;
            } else if (kp[0] == '/' && (kp[1] == ':' || kp[1] == '{')) {
                    // RESTful /:field/
                    // RESTful /{field}/
                    kp += 2;
                    ks = kp;
                    while (*kp && *kp != '/') {++kp;}
                    vp += 1;
                    vs = vp;
                    while (*vp && *vp != '/') {++vp;}
                    int klen = kp - ks;
                    if (*(ks-1) == '{' && *(kp-1) == '}') {
                        --klen;
                    }
                    Param p;
                    p.key = std::string(ks, klen);
                    p.value = std::string(vs, vp-vs);
                    vecParams.push_back(p);
                    continue;
            } else {
                ++kp;
                ++vp;
            }
        }

        match = match ? match : (*kp == '\0' && *vp == '\0');

        if (match) {
            if (vecParams.size() > 0) {
                params.assign(vecParams.begin(), vecParams.end());
            }
            return iter->get();
        }
    }

    return nullptr;
}

}