#include "CStaticPageRequestHandler.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

bool pathContainsFile(boost::filesystem::path dir, boost::filesystem::path file)
{
	// If dir ends with "/" and isn't the root directory, then the final
	// component returned by iterators will include "." and will interfere
	// with the std::equal check below, so we strip it before proceeding.
	if (dir.filename() == ".")
		dir.remove_filename();
	// We're also not interested in the file's name.
	assert(file.has_filename());
	file.remove_filename();

	// If dir has more components than file, then file can't possibly
	// reside in dir.
	auto dir_len = std::distance(dir.begin(), dir.end());
	auto file_len = std::distance(file.begin(), file.end());
	if (dir_len > file_len)
	{
		return false;
	}

	// This stops checking when it reaches dir.end(), so it's OK if file
	// has more directory components afterward. They won't be checked.
	return std::equal(dir.begin(), dir.end(), file.begin());
}

CStaticPageRequestHandler::CStaticPageRequestHandler()
{
	addFileNameAlias(".", "index.htm");
}

CStaticPageRequestHandler::~CStaticPageRequestHandler()
{
}

void CStaticPageRequestHandler::setRootDirectory(const char* directory)
{
	_rootDirectory =  boost::filesystem::absolute(boost::filesystem::path(directory));
}

void CStaticPageRequestHandler::addFileNameAlias(const char* alias, const char* realFileName)
{
	_aliasToRealFileName[std::string(alias)] = std::string(realFileName);
}

bool CStaticPageRequestHandler::validPath(const char* path, const char* method)
{
	if (strcmp(method, "GET") == 0)
	{
		boost::filesystem::path requestedPath = boost::filesystem::absolute(_rootDirectory / path);
		requestedPath = substituteAlias(requestedPath);
		if(!pathContainsFile(_rootDirectory, requestedPath))
		{
			return false;
		}
		if (boost::filesystem::exists(requestedPath))
		{
			return true;
		}
	}
	return false;
}

int CStaticPageRequestHandler::handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, uint32_t* upload_data_size)
{
	boost::filesystem::path requestedPath = boost::filesystem::absolute(_rootDirectory / url);
	requestedPath = substituteAlias(requestedPath);

	struct stat64 buf;
	if (stat64(requestedPath.string().c_str(), &buf) == -1)
	{
		return MHD_NO;
	}

	int fd = open(requestedPath.string().c_str(), O_RDONLY);

	struct MHD_Response * response = MHD_create_response_from_fd(buf.st_size, fd);

	int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

boost::filesystem::path& CStaticPageRequestHandler::substituteAlias(boost::filesystem::path& path)
{
	boost::filesystem::path fileName = path.filename();

	auto itr = _aliasToRealFileName.find(fileName.string());
	if(itr != _aliasToRealFileName.end())
	{
		path = path.parent_path() / itr->second;
	}

	return path;
}
