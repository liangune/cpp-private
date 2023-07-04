#ifndef _HTTP_TYPEDEF_H_
#define _HTTP_TYPEDEF_H_

#include <stdint.h>
#include <map>

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

// Integer limit values.
#define intSize   (32 << (^uint(0) >> 63)) // 32 or 64
#define MaxInt    (1<<(intSize-1) - 1)
#define MinInt    (-1 << (intSize - 1))
#define MaxInt8   ((1<<7) - 1)
#define MinInt8   (-1 << 7)
#define MaxInt16  (1<<15 - 1)
#define MinInt16  (-1 << 15)
#define MaxInt32  (1<<31 - 1)
#define MinInt32  (-1 << 31)
#define MaxInt64  (1<<63 - 1)
#define MinInt64  (-1 << 63)
#define MaxUint   (1<<intSize - 1)
#define MaxUint8  (1<<8 - 1)
#define MaxUint16 (1<<16 - 1)
#define MaxUint32 (1<<32 - 1)
#define MaxUint64 (1<<64 - 1)

using KeyValue = std::map<std::string, std::string>;

}

#endif // !_HTTP_TYPEDEF_H_