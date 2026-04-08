#ifndef _HTTP_MULTIPART_H_
#define _HTTP_MULTIPART_H_

#include "multipart_parser.h"
#include <string>
#include "stringUtils.h"

namespace workflowhttp {

enum multipart_parser_state_e {
    MP_START,
    MP_PART_DATA_BEGIN,
    MP_HEADER_FIELD,
    MP_HEADER_VALUE,
    MP_HEADERS_COMPLETE,
    MP_PART_DATA,
    MP_PART_DATA_END,
    MP_BODY_END
};

/**************multipart/form-data*************************************
--boundary
Content-Disposition: form-data; name="user"

content
--boundary
Content-Disposition: form-data; name="avatar"; filename="user.jpg"
Content-Type: image/jpeg

content
--boundary--
***********************************************************************/
struct Form {
    std::string     filename;
    std::string     content;
    std::string     contentType;

    Form(const char* content = NULL, const char* filename = NULL) {
        if (content) {
            this->content = content;
        }
        if (filename) {
            this->filename = filename;
        }
    }
    template<typename T>
    Form(T num) {
        content = StringUtils::to_string(num);
    }
};

typedef std::map<std::string, Form> MultiPartForm;

struct multipart_parser_userdata {
    MultiPartForm* mp;
    // tmp
    multipart_parser_state_e state;
    std::string headerField;
    std::string headerValue;
    std::string partData;
    std::string name;
    std::string filename;
    std::string contentType;

    void handle_header() {
        if (headerField.size() == 0 || headerValue.size() == 0) return;
        if (stricmp(headerField.c_str(), "Content-Disposition") == 0) {
            StringList strlist = StringUtils::split(headerValue, ';');
            for (auto& str : strlist) {
                StringList kv = StringUtils::split(StringUtils::trim(str, " "), '=');
                if (kv.size() == 2) {
                    const char* key = kv.begin()->c_str();
                    std::string value = *(kv.begin() + 1);
                    value = StringUtils::trim_pairs(value, "\"\"\'\'");
                    if (strcmp(key, "name") == 0) {
                        name = value;
                    }
                    else if (strcmp(key, "filename") == 0) {
                        filename = value;
                    }
                }
            }
        }

        if (stricmp(headerField.c_str(), "Content-Type") == 0) {
            contentType = headerValue;
        }

        headerField.clear();
        headerValue.clear();
    }

    void handle_data() {
        if (name.size() != 0) {
            Form formdata;
            formdata.content = partData;
            formdata.filename = filename;
            formdata.contentType = contentType; 
            (*mp)[name] = formdata;
        }
        name.clear();
        filename.clear();
        partData.clear();
        contentType.clear();
    }
};

class Multipart
{
public:
    static int OnHeaderField(multipart_parser* parser, const char *at, size_t length);
    static int OnHeaderValue(multipart_parser* parser, const char *at, size_t length);
    static int OnPartData(multipart_parser* parser, const char *at, size_t length);
    static int OnPartDataBegin(multipart_parser* parser);
    static int OnHeadersComplete(multipart_parser* parser);
    static int OnPartDataEnd(multipart_parser* parser);
    static int OnBodyEnd(multipart_parser* parser);

    static int ParseMultipart(const std::string& str, MultiPartForm& mp, const char* boundary);
};

}

#endif //_HTTP_MULTIPART_H_