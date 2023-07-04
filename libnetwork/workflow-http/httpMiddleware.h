#ifndef _HTTP_MIDDLEWARE_H_
#define _HTTP_MIDDLEWARE_H_

#include "context.h"

namespace workflowhttp {

class HttpMiddleware {
public:
    static void CORS(Context * ctx);
};

}
#endif // !_HTTP_MIDDLEWARE_H_
