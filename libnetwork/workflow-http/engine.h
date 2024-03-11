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
    Engine(unsigned int port, const std::string& host = "0.0.0.0");

    ~Engine();

    bool Run();
    bool Start();
    void Stop();

    // Start starts listening and serving HTTP requests.
    bool Start(const std::string& host, unsigned int port);

    // StartTLS starts listening and serving HTTPS (secure) requests.
    bool StartTLS(const std::string& host, unsigned int port, const std::string &certFile, const std::string &keyFile);

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

    // https://developer.mozilla.org/en-US/docs/Web/HTTP/CORS
    void AllowCORS();
};

typedef std::shared_ptr<Engine> EnginePtr;

}

#endif // !_HTTP_ENGINE_H_