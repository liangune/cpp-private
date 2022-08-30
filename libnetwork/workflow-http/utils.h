#ifndef _HTTP_UTILS_H_
#define _HTTP_UTILS_H_

#include <string>
#include <filesystem>

namespace workflowhttp {

class Utils {
public:
	static std::string JoinPaths(const std::string& absolutePath, const std::string& relativePath) {
		if(relativePath == "") {
			return absolutePath;
		}

		std::experimental::filesystem::path path(absolutePath);
		path /= relativePath;

		std::string finalPath = path.string();
		if(relativePath[relativePath.length() - 1] == '/' && finalPath[finalPath.length() - 1] != '/') {
			return finalPath + "/";
		}

		return finalPath;
	}

};

}


#endif //!_HTTP_UTILS_H_
