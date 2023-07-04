#include "engine.h"
#include "httpMiddleware.h"

namespace workflowhttp {

Engine::Engine(int port, const std::string& host) : port(port), host(host) {

}

Engine::~Engine() {
    Stop();
}

bool Engine::Start() {
    auto f = std::bind(&Engine::WFHandleHTTPRequest, this, std::placeholders::_1);
    wfHttpServer = std::make_shared<WFHttpServer>(f);
    if (wfHttpServer->start(host.c_str(), port) == 0){
        RebuildHandlers();
        return true;
    }

    return false;
}

void Engine::Stop() {
    wfHttpServer->stop();
}

Engine& Engine::Use(HandlerFunc handler) {
    handlers.emplace_back(std::move(handler));
    return *this;
}

// workflow HTTP request entry 
void Engine::WFHandleHTTPRequest(WFHttpTask *pHttpTask) {
    protocol::HttpRequest *req = pHttpTask->get_req();
    protocol::HttpResponse *resp = pHttpTask->get_resp();

    // remote addr
    char remoteHost[128];
    struct sockaddr_storage addr;
    socklen_t len = sizeof addr;
    unsigned short remotePort = 0;

    pHttpTask->get_peer_addr((struct sockaddr *)&addr, &len);
    if (addr.ss_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)&addr;
        inet_ntop(AF_INET, &sin->sin_addr, remoteHost, 128);
        remotePort = ntohs(sin->sin_port);
    }
    else if (addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&addr;
        inet_ntop(AF_INET6, &sin6->sin6_addr, remoteHost, 128);
        remotePort = ntohs(sin6->sin6_port);
    }

    std::string host = remoteHost + std::string(":") + std::to_string(this->port);
    ContextPtr ctx(std::make_shared<Context>(req, resp));
    /*
    Context *ctx = new Context(req, resp);
    ctx->Init(remoteHost, remotePort, host);

    HandleHTTPRequest(ctx);
    delete ctx;
    */
    ctx.get()->Init(remoteHost, remotePort, host);

    HandleHTTPRequest(ctx.get());
}

// HTTP request handle
void Engine::HandleHTTPRequest(Context *ctx) {
    std::string httpMethod = ctx->Method();
    std::string path = ctx->Path();

    auto it = methodHandlerFuncMap.find(httpMethod);
    if(it == methodHandlerFuncMap.end()) {
        ctx->CombineHandlers(allNoMethod);
        ServeError(ctx, StatusMethodNotAllowed, default405Body);
        return;
    }

    RouterNode* pRouterNode = GetRouterNode(path, it->second, ctx->params);
    if(pRouterNode == nullptr) {
        ctx->CombineHandlers(allNoRoute);
        ServeError(ctx, StatusNotFound, default404Body);
        return;
    }

    auto f = pRouterNode->handler;
    if (f == nullptr) {
        ctx->CombineHandlers(allNoRoute);
        ServeError(ctx, StatusNotFound, default404Body);
        return;
    }

    ctx->CombineHandlers(handlers);
    ctx->CombineHandler(f);
    ctx->Next();
}

void Engine::RebuildHandlers() {
    Rebuild404Handlers();
    Rebuild405Handlers();
}

void Engine::Rebuild404Handlers() {
    CombineHandlers(noRoute, allNoRoute);
}

void Engine::Rebuild405Handlers() {
    CombineHandlers(noMethod, allNoMethod);
}

// HTTP response
void Engine::ServeError(Context *ctx, int code, const std::string &defaultMessage) {
    ctx->SetStatus(code);
    ctx->Next();

    ctx->SetHeader("Content-Type", MIMEPlain);
    ctx->Write(defaultMessage);
}

void Engine::AllowCORS() {
    Use(HttpMiddleware::CORS);
}

}