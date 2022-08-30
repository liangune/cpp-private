#ifndef _HTTP_ROUTER_NODE_H_
#define _HTTP_ROUTER_NODE_H_
#include "context.h"
#include <stdint.h>

namespace workflowhttp {

#define maxParamCount 255

// 节点类型: static, param, catchAll
// static: 静态节点
// catchAll: 有*匹配的节点
// param: 参数节点
enum NodeType {
    NodeTypeStatic = 0,    
    NodeTypeParam = 1,
    NodeTypeCatchAll = 2,
};

class RouterNode {
public:
	std::string path;
	HandlerFunc handler;
    bool wildChild;
    uint32_t maxParams;
    NodeType nType;

public:
    RouterNode() = default;
    ~RouterNode() = default;

    uint8_t CountParams(const std::string &path) {
        uint8_t n = 0;
        for (int i = 0; i < path.size(); i++) {
            if (path[i] != ':' && path[i] != '*') {
                continue;
            }
            n++;
        }
        if (n >= maxParamCount) {
            return maxParamCount;
        }
        return n;
    }

    RouterNode(const std::string &path, HandlerFunc handler) {
        this->handler = handler;
        this->wildChild = false;
        std::string fullPath = path;
        this->maxParams = CountParams(path);

        if(this->maxParams > 0) {
            for(size_t i = 0; i < path.size(); i++) {
                auto c = path[i];
                if(c == ':') {
                    this->path = std::string(path.c_str(), i);
                    this->nType = NodeTypeParam;
                    break;
                }
                if(c == '*') {
                    this->path = std::string(path.c_str(), i);
                    this->nType = NodeTypeCatchAll;
                    this->wildChild = true;
                    break;
                }
            }
        } else {
            this->path = path;
            this->nType = NodeTypeStatic;
        }
    }
};

}


#endif // !_HTTP_ROTER_NODE_H_