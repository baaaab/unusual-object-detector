#ifndef CDISPLAY_H_
#define CDISPLAY_H_

#include <ILiveResultManager.h>

#include <SDL/SDL.h>
#include <mutex>
#include <opencv2/opencv.hpp>

class CModel;
class CImageStore;
class IJpegHandler;
class CScoreDistribution;
class CThread;

class CDisplay: public ILiveResultManager
{
public:
	CDisplay(CImageStore* imageStore, CScoreDistribution* scoresDistribution);
	virtual ~CDisplay();

	void setSourceImage(uint32_t imageId, cv::Mat sourceImage);
	void setMatchImage(uint32_t imageId, float score, bool isUnusual);

private:

	static void DisplayThread(void* arg);
	void displayThread();

	void blit(cv::Mat image, int x, int y);
	void drawRectangle(int x1, int y1, int x2, int y2, uint32_t pixel);
	void setPixel(int x, int y, uint32_t pixel);
	uint32_t setColour(uint8_t r, uint8_t g, uint8_t b);
	void drawModel(CModel* f);
	void drawQuadrants(int level, int sqx, int sqy);
	void drawStraightLine(int x1, int y1, int x2, int y2, uint32_t pixel);

	void drawScoreDistribution();

	bool _shutdownRequested;

	SDL_Surface* _screen;
	SDL_Event _event;
	uint32_t _windowWidth;
	uint32_t _windowHeight;

	IJpegHandler* _jpegHandler;
	CImageStore* _imageStore;
	CScoreDistribution* _scoresDistribution;

	uint32_t _matchedImageId;
	float _matchedImageScore;
	bool _matchedImageIsUnusual;
	bool _matchedImageNeedsReloading;

	cv::Mat _sourceImage;
	cv::Mat _matchedImage;

	bool _firstImageReceived;
	bool _firstMatchReceived;

	CThread* _thread;
	std::recursive_mutex _mutex;

	uint8_t _loadingCounter;

};

#endif /* CDISPLAY_H_ */