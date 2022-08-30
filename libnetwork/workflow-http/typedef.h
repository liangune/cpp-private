#ifndef _HTTP_TYPEDEF_H_
#define _HTTP_TYPEDEF_H_

namespace workflowhttp {

#define default404Body  "404 page not found"
#define default405Body  "405 method not allowed"

// Content-Type MIME of the most common data formats.
#define MIMEJSON                "application/json"
#define MIMEHTML                "text/html"
#define MIMEXML                 "application/xml"
#define MIMEXML2                "text/xml"
#define MIMEPlain               "text/plain"
#define MIMEPOSTForm            "application/x-www-form-urlencoded"
#define MIMEMultipartPOSTForm   "multipart/form-data"
#define MIMEPROTOBUF            "application/x-protobuf"
#define MIMEMSGPACK             "application/x-msgpack"
#define MIMEMSGPACK2            "application/msgpack"
#define MIMEYAML                "application/x-yaml"


}

#endif // !_HTTP_TYPEDEF_H_