#include "context.h"
#include "workflow/RWLock.h"
#include "workflow/WFTaskFactory.h"
#include "workflow/HttpMessage.h"

namespace workflowhttp {

Context::Context() : request(nullptr), response(nullptr), header(nullptr) {
    index = -1;
    fullPath = "";
    isRequestBody = false;
    isQueryCache = false;
}

Context::Context(protocol::HttpRequest *req, protocol::HttpResponse *resp) : header(nullptr) {
    request = req;
    response = resp;
    index = -1;
    fullPath = "";
    isRequestBody = false;
    isQueryCache = false;
}

Context::~Context() {
    if(header != nullptr) {
        delete header;
        header = nullptr;
    }
}

void Context::Init(const char *remoteHost, uint32_t port, std::string &host) {	    
    method = request->get_method();
    requestURI = request->get_request_uri();
    
    this->host = host;
    clientIP = remoteHost;
    remoteAddr = clientIP + std::string(":") + std::to_string(port);
    
    std::string allPath = "http://" + host + requestURI;
    if(URL.Parse(allPath) < 0) {
		Error("HTTP request URL parse failed.");
    }
    fullPath = URL.path;
}

// WriteStatus sets the HTTP response code.
void Context::WriteStatus(int code) {
    statusCode = code;
    response->set_status_code(std::to_string(statusCode));
}

// Write sets the HTTP response content.
bool Context::Write(const std::string& buf) {
    return response->append_output_body(buf);
}

 // Status gets the HTTP response code.
int Context::Status() const {
    return statusCode;
}

// It writes a header in the response.
bool Context::WriteHeader(const std::string &key, const std::string &value) {
    return response->add_header_pair(key.c_str(), value.c_str());
}


// GetHeader returns value from request headers.
std::string Context::GetHeader(const std::string &key) {
    if(header == nullptr) {
        header = new protocol::HttpHeaderMap(request);
    }
    return header->get(key); 
}

// get HTTP request method.
std::string Context::Method() const {
    return method;
}

std::string Context::RequestURI() const {
    return requestURI;
}

std::string Context::ClientIP() const {
    return clientIP;
}

std::string Context::Proto() {
    if(proto == "") {
        request->get_http_version(proto);
    }
    return proto;
}

std::string Context::Path() const {
    return URL.path;
}

Slice& Context::GetBody() {
    if(!isRequestBody) {
		const void *body;
        size_t len;
        if(request->get_parsed_body(&body, &len)) {
            reqestBody = Slice((char *)body, len);
            isRequestBody = true;
        }
    }

    return reqestBody;
}

std::string Context::Referer() {
    return GetHeader("Referer");
}

// it returns the keyed url query value
// if it exists `value ` (even when the value is an empty string),
// otherwise it returns empty.
//     GET /?name=Manu&lastname=
//     ("Manu") == c.GetQuery("name")
//     ("") == c.GetQuery("id")
//     ("") == c.GetQuery("lastname")
std::string Context::GetQuery(const std::string &key) {
    std::vector<std::string> values;
    if(GetQueryArray(key, values)) {
        return values[0];
    }
    return "";
}

// GetQueryArray returns a vector of strings for a given query key, plus
// a boolean value whether at least one value exists for the given key.
bool Context::GetQueryArray(const std::string &key, std::vector<std::string> &values) {
    InitQueryCache();
    auto it = queryCache.find(key);
    if(it != queryCache.end() && it->second.size() > 0) {
        auto &val = it->second;
        values.assign(val.begin(), val.end());
        return true;
    }

    return false;
}

// Set is used to store a new key/value pair exclusively for this context.
void Context::Set(const std::string &key, std::string &value) {
    WriteLock lock(rwMutex);
    keys[key] = value;
}

bool Context::Get(const std::string &key, std::string &value) {
    ReadLock lock(rwMutex);
    bool exists = false;
    auto it = keys.find(key);
    if(it != keys.end()) {
        value = it->second;
        exists = true;
    }
    
    return exists;
}


void Context::InitQueryCache() {
    if(!isQueryCache) {
        std::string query = URL.query;
        auto q = URIParser::split_query_strict(query);
        queryCache.swap(q);
        isQueryCache = true;
    }
}

void Context::CombineHandlers(HandlersChain &handlers) {
    for(size_t i = 0; i < handlers.size(); i++) {
        this->handlers.push_back(handlers[i]);
    }
}

void Context::CombineHandler(HandlerFunc handler) {
    handlers.push_back(handler);
}

// Next should be used only inside middleware.
// It executes the pending handlers in the chain inside the calling handler.
void Context::Next() {
    index++;
    while (index < handlers.size()) {
        handlers[index](this);
        index++;
    }
}

// Error attaches an error to the current context. The error is pushed to a list of errors.
// It's a good idea to call Error for each error that occurred during the resolution of a request.
// A middleware can be used to collect all the errors and push them to a database together,
// print a log, or append it in the HTTP response.
void Context::Error(const std::string &errMsg) {
    if (errMsg == "") {
        return;
    }

    workflowhttp::Error err(errMsg);
    errors.emplace_back(err);
}

}