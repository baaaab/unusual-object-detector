#include "CDirectoryImageSource.h"

#include <boost/filesystem.hpp>
#include "CJpegHandlerFactory.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <IJpegHandler.h>
#include <sys/stat.h>

namespace
{
bool sortNumericFilename(std::string& a, std::string& b)
{
	boost::filesystem::path ap(a);
	boost::filesystem::path bp(b);

	uint32_t ai = strtoul(ap.filename().c_str(), NULL, 10);
	uint32_t bi = strtoul(bp.filename().c_str(), NULL, 10);
	return ai < bi;
}
}

CDirectoryImageSource::CDirectoryImageSource(const char* directory) :
		_jpegHandler( NULL )
{
	_jpegHandler = CJpegHandlerFactory::GetHandler();

	_files = getImages(directory);

	_files.sort(sortNumericFilename);
}

CDirectoryImageSource::~CDirectoryImageSource()
{
	delete _jpegHandler;
}

cv::Mat CDirectoryImageSource::getImage()
{
	if(_files.size() == 0)
	{
		throw 1;
	}
	std::string jpg = _files.front();
	_files.pop_front();

	struct stat st;
	if(stat(jpg.c_str(), &st) == 0)
	{
		FILE* fh = fopen(jpg.c_str(), "r");
		uint8_t* data = new uint8_t[st.st_size];
		fread(data, 1, st.st_size, fh);
		fclose(fh);
		cv::Mat mat = _jpegHandler->decompress(data, st.st_size);
		delete[] data;
		return mat;
	}
	else
	{
		throw 1;
	}
}

std::list<std::string> CDirectoryImageSource::getImages(const char* directory)
{
	boost::filesystem::path dir(directory);

	std::list<std::string> images;

	boost::filesystem::directory_iterator endIter;
	for (boost::filesystem::directory_iterator itr(dir); itr != endIter; ++itr)
	{
		if (boost::filesystem::is_directory(itr->status()))
		{
			std::list<std::string> subDirfiles = getImages(itr->path().string().c_str());
			images.insert(images.end(), subDirfiles.begin(), subDirfiles.end());
		}
		else
		{
			if (itr->path().string().find(".jpg") != std::string::npos)
			{
				images.push_back(itr->path().string());
			}
		}
	}

	return images;
}


