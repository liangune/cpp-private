#ifndef _HTTP_ERROR_H_
#define _HTTP_ERROR_H_

#include <string>

namespace workflowhttp {

class Error {
private:
    std::string err;
public:
    Error() = default;
    Error(const std::string &errorMsg) {
        err = errorMsg;
    }
    ~Error() = default;

    std::string String() {
        return err;
    }
};

}

#endif // !_HTTP_ERROR_H_