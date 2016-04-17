#ifndef SRC_HTTP_CSTATICPAGEREQUESTHANDLER_H_
#define SRC_HTTP_CSTATICPAGEREQUESTHANDLER_H_

#include "http/IhttpRequestHandler.h"

#include <boost/filesystem.hpp>
#include <map>
#include <string>

class CStaticPageRequestHandler: public IHttpRequestHandler
{
public:
	CStaticPageRequestHandler();
	virtual ~CStaticPageRequestHandler();

	void setRootDirectory(const char* directory);
	void addFileNameAlias(const char* alias, const char* realFileName);

	bool validPath(const char* path, const char* method);
	int handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, uint32_t* upload_data_size);

private:
	boost::filesystem::path& substituteAlias(boost::filesystem::path& path);

	boost::filesystem::path _rootDirectory;
	std::map<std::string, std::string> _aliasToRealFileName;
};

#endif /* SRC_HTTP_CSTATICPAGEREQUESTHANDLER_H_ */
