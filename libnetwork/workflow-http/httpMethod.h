#ifndef _HTTP_METHOD_H_
#define _HTTP_METHOD_H_

#include <map>
#include <string>
namespace workflowhttp {

// Common HTTP methods.
//
// Unless otherwise noted, these are defined in RFC 7231 section 4.3.
#define MethodGet       "GET"
#define	MethodHead      "HEAD"
#define	MethodPost      "POST"
#define	MethodPut       "PUT"
#define	MethodPatch     "PATCH" // RFC 5789
#define	MethodDelete    "DELETE"
#define	MethodConnect   "CONNECT"
#define	MethodOptions   "OPTIONS"
#define	MethodTrace     "TRACE"

}

#endif // !_HTTP_METHOD_H_