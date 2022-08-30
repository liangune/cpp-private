#include "router.h"

namespace workflowhttp {

void Router::Handle(const std::string &httpMethod, const std::string &relativePath, HandlerFunc handler) {
    AddRoute(httpMethod, relativePath, handler);
}

void Router::CombineHandlers(HandlersChain &handlers, HandlersChain &mergedHandlers) {
    size_t finalSize = handlers.size() + this->handlers.size();

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

void Router::DELETE1(const std::string& relativePath , HandlerFunc handler) {
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

HandlerFunc Router::GetHandler(const std::string& path, HTTPHandlerFuncVec &handers) {
    for(auto &node : handers) {
        if(node->nType == workflowhttp::NodeTypeStatic) {
            if(path == node->path) {
                return node->handler;
            }
        } else if(node->nType == workflowhttp::NodeTypeParam) {
            if(path.size() >= node->path.size()) {
                std::string prefix(path.c_str(), node->path.size());
                if(node->path == prefix) {
                    return node->handler;
                }
            }
        } else {
            if(path.size() >= node->path.size()) {
                std::string prefix(path.c_str(), node->path.size());
                if(node->path == prefix) {
                    return node->handler;
                }
            }
        }
    }

    return nullptr;
}

}