#include "CImageStore.h"

#include <CJpegHandlerFactory.h>
#include <sys/stat.h>
#include <dirent.h>

CImageStore::CImageStore(CSettingsRegistry* registry) :
		_jpegHandler( NULL)
{
	_jpegHandler = CJpegHandlerFactory::GetHandler();
	_imageDir = registry->getString("core", "imageDir");
}

CImageStore::~CImageStore()
{
	delete _jpegHandler;
}

void CImageStore::saveImage(cv::Mat image, uint32_t programCounter)
{
	uint32_t imageDirNumber = programCounter / 100;
	std::string imageSubDir = _imageDir;
	imageSubDir.append("/images/").append(std::to_string(imageDirNumber)).append("/");

	struct stat pathInfo;
	if (stat(imageSubDir.c_str(), &pathInfo) == -1)
	{
		mkdir(imageSubDir.c_str(), 0775);
	}

	std::string filename = imageSubDir + std::to_string(programCounter) + std::string(".jpg");

	std::vector<unsigned char> jpeg = _jpegHandler->compress(image);

	FILE* fh = fopen(filename.c_str(), "wb");
	uint32_t bytesWritten = 0;
	uint32_t totalBytesWritten = 0;
	while((bytesWritten = fwrite(&jpeg[totalBytesWritten], 1, jpeg.size() - totalBytesWritten, fh)) > 0)
	{
		totalBytesWritten += bytesWritten;
	}
	fclose(fh);
}

std::string CImageStore::fetchImagePath(uint32_t programCounter)
{
	uint32_t imageDirNumber = programCounter / 100;
	std::string imageSubDir = _imageDir;
	imageSubDir.append("/images/").append(std::to_string(imageDirNumber)).append("/");

	std::string filename = imageSubDir.append(std::to_string(programCounter)).append(".jpg");

	return filename;
}

cv::Mat CImageStore::fetchImage(uint32_t programCounter)
{
	std::string filename = fetchImagePath(programCounter);

	FILE* fh = fopen(filename.c_str(), "rb");
	if (fh == NULL)
	{
		printf("%s::%s Image: %s is missing!\n", __FILE__, __FUNCTION__, filename.c_str());
		throw 1;
	}

	fseek(fh, 0, SEEK_END);
	uint32_t size = ftell(fh);
	fseek(fh, 0, SEEK_SET);

	unsigned char* buffer = new unsigned char[size];

	if(fread(buffer, 1, size, fh) != size)
	{
		printf("%s::%s Read error for file: %s\n", __FILE__, __FUNCTION__, filename.c_str());
		throw 1;
	}
	fclose(fh);

	cv::Mat im = _jpegHandler->decompress(buffer, size);

	delete[] buffer;

	return im;

}

std::vector<uint32_t> CImageStore::getUnusualImageList() const
{
	std::vector<uint32_t> ret;
	std::string unusualImagePath = _imageDir + "/unusual";
	DIR* dir = opendir(unusualImagePath.c_str());
	if(dir != NULL)
	{
		struct dirent* entry = NULL;
		while((entry = readdir(dir)) != NULL)
		{
			uint32_t id = strtoul(entry->d_name, NULL, 10);
			if(id)
			{
				ret.push_back(id);
			}
		}
		closedir(dir);
	}
	std::sort(ret.begin(), ret.end());
	return ret;

}

