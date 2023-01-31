#ifndef _HTTP_ENGINE_H_
#define _HTTP_ENGINE_H_

#include "router.h"
#include "context.h"
#include "typedef.h"
#include <memory>
#include "httpStatus.h"

#ifdef WIN32
#pragma comment(lib,"MSWSOCK.lib")
#pragma comment(lib,"ws2_32.lib") 
#pragma comment(lib,"kernel32.lib")
#else 
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

namespace workflowhttp {

class Engine : public Router {
private:
    WFHttpServerPtr wfHttpServer;
	int			port;
	std::string	host;

    HandlersChain allNoRoute;
    HandlersChain allNoMethod;
    HandlersChain noRoute;
	HandlersChain noMethod;

public:
    Engine() = default;
    Engine(int port, const std::string& host = "0.0.0.0");

    ~Engine();

    bool Start();
    void Stop();

    // Use attaches a global middleware to the router. ie. the middleware attached though Use() will be
    // included in the handlers chain for every single request. Even 404, 405, static files...
    // For example, this is the right place for a logger or error management middleware.
    Engine& Use(HandlerFunc handler);

    // workflow HTTP request entry 
    void WFHandleHTTPRequest(WFHttpTask *pHttpTask);
    // HTTP request handle
    void HandleHTTPRequest(Context *ctx);

    void RebuildHandlers();
    void Rebuild404Handlers();
    void Rebuild405Handlers();

    // HTTP response
    void ServeError(Context *ctx, int code, const std::string &defaultMessage);

};

}

#endif // !_HTTP_ENGINE_H_