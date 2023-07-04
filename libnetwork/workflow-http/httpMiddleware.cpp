#include "httpMiddleware.h"
#include "httpMethod.h"
#include "httpStatus.h"

namespace workflowhttp {

void HttpMiddleware::CORS(Context * ctx) {

	ctx->SetHeader("Access-Control-Allow-Origin", "*");
	ctx->SetHeader("Access-Control-Allow-Headers", "Content-Type, AccessToken, X-CSRF-Token, Authorization, Token, x-token");
	ctx->SetHeader("Access-Control-Allow-Methods", "OPTIONS, HEAD, GET, POST, PUT, DELETE, PATCH");
    //ctx->SetHeader("Access-Control-Expose-Headers", "Content-Length, Access-Control-Allow-Origin, Access-Control-Allow-Headers, Content-Type");
    //ctx->SetHeader("Access-Control-Allow-Credentials", "true");

    if (ctx->Method() == MethodOptions) {
		ctx->AbortWithStatus(StatusNoContent);
    }
}

}