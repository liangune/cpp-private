
#include "multipart.h"

namespace workflowhttp {
int Multipart::OnHeaderField(multipart_parser* parser, const char *at, size_t length) {
    multipart_parser_userdata* userdata = (multipart_parser_userdata*)multipart_parser_get_data(parser);
    userdata->handle_header();
    userdata->state = MP_HEADER_FIELD;
    userdata->headerField.append(at, length);
    return 0;
}

int Multipart::OnHeaderValue(multipart_parser* parser, const char *at, size_t length) {
    multipart_parser_userdata* userdata = (multipart_parser_userdata*)multipart_parser_get_data(parser);
    userdata->state = MP_HEADER_VALUE;
    userdata->headerValue.append(at, length);
    return 0;
}

int Multipart::OnPartData(multipart_parser* parser, const char *at, size_t length) {
    multipart_parser_userdata* userdata = (multipart_parser_userdata*)multipart_parser_get_data(parser);
    userdata->state = MP_PART_DATA;
    userdata->partData.append(at, length);
    return 0;
}

int Multipart::OnPartDataBegin(multipart_parser* parser) {
    multipart_parser_userdata* userdata = (multipart_parser_userdata*)multipart_parser_get_data(parser);
    userdata->state = MP_PART_DATA_BEGIN;
    return 0;
}

int Multipart::OnHeadersComplete(multipart_parser* parser) {
    multipart_parser_userdata* userdata = (multipart_parser_userdata*)multipart_parser_get_data(parser);
    userdata->handle_header();
    userdata->state = MP_HEADERS_COMPLETE;
    return 0;
}

int Multipart::OnPartDataEnd(multipart_parser* parser) {
    multipart_parser_userdata* userdata = (multipart_parser_userdata*)multipart_parser_get_data(parser);
    userdata->state = MP_PART_DATA_END;
    userdata->handle_data();
    return 0;
}
int Multipart::OnBodyEnd(multipart_parser* parser) {
    multipart_parser_userdata* userdata = (multipart_parser_userdata*)multipart_parser_get_data(parser);
    userdata->state = MP_BODY_END;
    return 0;
}

int Multipart::ParseMultipart(const std::string& str, MultiPartForm& mp, const char* boundary) {
    std::string __boundary("--");
    __boundary += boundary;
    multipart_parser_settings settings;
    settings.on_header_field = OnHeaderField;
    settings.on_header_value = OnHeaderValue;
    settings.on_part_data    = OnPartData;
    settings.on_part_data_begin  = OnPartDataBegin;
    settings.on_headers_complete = OnHeadersComplete;
    settings.on_part_data_end    = OnPartDataEnd;
    settings.on_body_end         = OnBodyEnd;
    multipart_parser* parser = multipart_parser_init(__boundary.c_str(), &settings);
    multipart_parser_userdata userdata;
    userdata.state = MP_START;
    userdata.mp = &mp;
    multipart_parser_set_data(parser, &userdata);
    size_t nparse = multipart_parser_execute(parser, str.c_str(), str.size());
    multipart_parser_free(parser);
    return nparse == str.size() ? 0 : -1;
}

}