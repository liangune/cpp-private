#ifndef _HTTP_CONTEXT_H_
#define _HTTP_CONTEXT_H_

#include "workflow/WFHttpServer.h"
#include "workflow/HttpUtil.h"
#include <string>
#include "typedef.h"
#include "slice.h"
#include <memory>
#include <functional>
#include <stdint.h>
#include "urlParser.h"
#include "error.h"
#include "workflow/RWLock.h"
#include "httpCookie.h"

namespace workflowhttp {
class Context;

using WFHandleFunc = std::function<void(WFHttpTask * httpTask)>;
using HandlerFunc = std::function<void(Context * ctx)>;
using HandlersChain = std::vector<HandlerFunc>;

using URLValues = std::map<std::string, std::vector<std::string>>;
using WFHttpServerPtr = std::shared_ptr<WFHttpServer>;
using ContextPtr = std::shared_ptr<Context>;
using ErrorsChain = std::vector<Error>;

#define abortIndex MaxInt8 / 2

// Param is a single URL parameter, consisting of a key and a value.
typedef struct {
	std::string key;
	std::string value;
}Param;

// Params is a Param-slice, as returned by the router.
// The slice is ordered, the first URL parameter is also the first slice value.
// It is therefore safe to read values by the index.
using Params = std::vector<Param>;

class Context {
public:
    protocol::HttpRequest *request;
    protocol::HttpResponse *response;
    HandlersChain handlers;
    Params params;

private:
    size_t index;
    std::string fullPath;

    // method specifies the HTTP method (GET, POST, PUT, etc.).
	// For client requests, an empty string means GET.
    std::string method;

    // URL specifies either the URI being requested (for server
	// requests) or the URL to access (for client requests).
    URLParser URL;

    // "HTTP/1.0"
    std::string proto;

    // HTTP header
    protocol::HttpHeaderMap *header;

    // The HTTP request body data
    Slice reqestBody;
    bool isRequestBody;
    
    // requestURI is the unmodified request-target of the
	// Request-Line (RFC 7230, Section 3.1.1) as sent by the client
	// to a server. Usually the URL field should be used instead.
	// It is an error to set this field in an HTTP client request.
    std::string requestURI;
    
    std::string host;
    // remoteAddr allows HTTP servers and other software to record
	// sets remoteAddr to an "IP:port" address 
	// This field is ignored by the HTTP client.
    std::string remoteAddr;
    std::string clientIP;

    // queryCache cached the param query result from HTTP request
    URLValues queryCache;
    bool isQueryCache;
    
    // keys is a key/value pair exclusively for the context of each request.
	std::map<std::string, std::string> keys;
    
    // This mutex protect keys map
    RWLock rwMutex;

    // errors is a list of errors attached to all the handlers/middlewares who used this context.
    ErrorsChain errors;

    // the HTTP response code
    int statusCode;
    
public:
    Context();
    Context(protocol::HttpRequest *req, protocol::HttpResponse *resp);
    ~Context();

    void Init(const char *remoteHost, uint32_t port, std::string &host);

    // SetStatus sets the HTTP response code.
    void SetStatus(int code);

    // Write sets the HTTP response content.
    bool Write(const std::string& buf);
    bool Write(const char *pszBuf, size_t nSize);

    // GetStatus gets the HTTP response code.
    int GetStatus() const;

    // It writes a header in the response.
    bool SetHeader(const std::string &key, const std::string &value);

    // GetHeader returns value from request headers.
    std::string GetHeader(const std::string &key);

    // get HTTP request method.
    std::string Method() const;

    std::string RequestURI() const;

    std::string ClientIP() const;

    std::string Proto();

    std::string Path() const;

    Slice& GetBody();
    std::string GetBodyString();

    // Referer is misspelled as in the request itself, a mistake from the
    // earliest days of HTTP.  This value can also be fetched from the
    // Header map as Header["Referer"]; the benefit of making it available
    // as a method is that the compiler can diagnose programs that use the
    // alternate (correct English) spelling req.Referrer() but cannot
    // diagnose programs that use Header["Referrer"].
    std::string Referer();

    // GetParam returns the value of the URL param.
    // It is a shortcut for c.Params.ByName(key)
    //     router.GET("/user/:id", [&](workflowhttp::Context *ctx) {
    //         // a GET request to /user/john
    //         id := ctx->GetParam("id") // id == "john"
    //     })
    std::string GetParam(const std::string &key);

    // it returns the keyed url query value
    // if it exists `value ` (even when the value is an empty string),
    // otherwise it returns empty.
    //     GET /?name=Manu&lastname=
    //     ("Manu") == c.GetQuery("name")
    //     ("") == c.GetQuery("id")
    //     ("") == c.GetQuery("lastname")
    std::string GetQuery(const std::string &key);

    // GetQueryArray returns a vector of strings for a given query key, plus
    // a boolean value whether at least one value exists for the given key.
    bool GetQueryArray(const std::string &key, std::vector<std::string> &values);

    // Set is used to store a new key/value pair exclusively for this context.
    void Set(const std::string &key, std::string &value);

    bool Get(const std::string &key, std::string &value);

    void InitQueryCache();

    void CombineHandlers(HandlersChain &handlers);

    void CombineHandler(HandlerFunc handler);

    // Next should be used only inside middleware.
    // It executes the pending handlers in the chain inside the calling handler.
    void Next();

    // Error attaches an error to the current context. The error is pushed to a list of errors.
    // It's a good idea to call Error for each error that occurred during the resolution of a request.
    // A middleware can be used to collect all the errors and push them to a database together,
    // print a log, or append it in the HTTP response.
    void Error(const std::string& errMsg);

    // IsAborted returns true if the current context was aborted.
    bool IsAborted();
    
    void Abort(); 
    
    void AbortWithStatus(int code);

    void SetCookie(const HttpCookie& cookie);
    void SetCookie(const std::string &name, const std::string &value, int maxAge, const std::string& path, const std::string &domain, bool secure, bool httpOnly);
};

}

#endif // !_HTTP_CONTEXT_H_