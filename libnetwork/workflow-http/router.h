#ifndef _HTTP_ROUTRE_GROUP_H_
#define _HTTP_ROUTRE_GROUP_H_

#include "context.h"
#include "httpMethod.h"
#include "routerNode.h"

namespace workflowhttp {

using HTTPHandlerFuncVec = std::vector<std::shared_ptr<RouterNode>>;
using HTTPMethodHandlerFuncMap = std::map<std::string/*The method*/, HTTPHandlerFuncVec>;

class IRouter {
public:
	virtual void Handle(const std::string& httpMethod, const std::string& relativePath, HandlerFunc handler) = 0;
	virtual void GET(const std::string& relativePath, HandlerFunc handler) = 0;
	virtual void POST(const std::string& relativePath, HandlerFunc handler) = 0;
	virtual void Delete(const std::string& relativePath, HandlerFunc handler) = 0;
	virtual void PATCH(const std::string& relativePath, HandlerFunc handler) = 0;
	virtual void PUT(const std::string& relativePath, HandlerFunc handler) = 0;
	virtual void OPTIONS(const std::string& relativePath, HandlerFunc handler) = 0; 
	virtual void HEAD(const std::string& relativePath, HandlerFunc handler) = 0;
    virtual void Any(const std::string& relativePath, HandlerFunc handler) = 0;
};


class Router : public IRouter {
protected:
	HandlersChain handlers;
    HTTPMethodHandlerFuncMap methodHandlerFuncMap;
    std::string baseUrl;

public:
    Router() = default;
    ~Router() = default;

    // Handle registers a new request handle with the given path and method.
    // For GET, POST, PUT, PATCH and DELETE requests the respective shortcut
    // functions can be used.
    void Handle(const std::string &httpMethod, const std::string &relativePath, HandlerFunc handler);
    void AddRoute(const std::string &httpMethod, const std::string &path, HandlerFunc handler);

    // The last handler should be the real handler, the other ones should be middleware that can and should be shared among different routes.
    void CombineHandlers(HandlersChain &handlers, HandlersChain &mergedHandlers);
    
    void GET(const std::string& relativePath , HandlerFunc handler);
    void POST(const std::string& relativePath , HandlerFunc handler);
    void Delete(const std::string& relativePath , HandlerFunc handler);
    void PATCH(const std::string& relativePath , HandlerFunc handler);
    void PUT(const std::string& relativePath , HandlerFunc handler);
    void OPTIONS(const std::string& relativePath , HandlerFunc handler);
    void HEAD(const std::string& relativePath , HandlerFunc handler);
    void Any(const std::string& relativePath, HandlerFunc handler);
    
    // RESTful API
    // GetRouterNode is a matching handler function
    RouterNode* GetRouterNode(const std::string& path, HTTPHandlerFuncVec &handers, Params &params);

private:
    bool Strendswith(const char* str, const char* end);
};

}

#endif // !_HTTP_ROUTRE_GROUP_H_