#include "CDisplay.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <cmath>
#include <unistd.h>
#include <limits>
#include <algorithm>

#include <settings.h>
#include "../utils/CThread.h"
#include "CAutoScreenLock.h"
#include "../core/CModel.h"
#include "../core/CImageStore.h"
#include <IJpegHandler.h>
#include "CJpegHandlerFactory.h"
#include "../core/CScoreDistribution.h"
#include "../core/CHog.h"

namespace
{
const uint32_t IMAGE_WIDTH = 512;
const uint32_t IMAGE_HEIGHT = 512;

typedef std::lock_guard<std::recursive_mutex> AutoMutex;
}

CDisplay::CDisplay(CImageStore* imageStore, CScoreDistribution* scoresDistribution, CModel* model) :
		_shutdownRequested(false),
		_screen( NULL),
		_windowWidth(IMAGE_WIDTH * 2),
		_windowHeight(IMAGE_HEIGHT + 140),
		_jpegHandler( NULL),
		_imageStore(imageStore),
		_scoresDistribution(scoresDistribution),
		_model(model),
		_matchedImageId(0xffffffff),
		_matchedImageIsUnusual(false),
		_matchedImageNeedsReloading(true),
		_firstImageReceived(false),
		_firstMatchReceived(false),
		_thread( NULL),
		_loadingCounter(0),
		_drawType(DRAW_IMAGE)
{
	XInitThreads();

	if (SDL_Init( SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr, "Could not initialise SDL: %s\n", SDL_GetError());
	}

	if ((_screen = SDL_SetVideoMode(_windowWidth, _windowHeight, 32, SDL_SWSURFACE | SDL_DOUBLEBUF)) == NULL)
	{
		fprintf(stderr, "Could not set SDL video mode: %s\n", SDL_GetError());
		SDL_Quit();
		throw 1;
	}
	SDL_WM_SetCaption("Unusual Object Detector", "Unusual Object Detector");

	_jpegHandler = CJpegHandlerFactory::GetHandler();

	_thread = new CThread(DisplayThread, this);
}

CDisplay::~CDisplay()
{
	_shutdownRequested = true;
	delete _thread;
	delete _jpegHandler;
	_imageStore = NULL;
	SDL_Quit();
}

void CDisplay::setSourceImage(uint32_t imageId, cv::Mat sourceImage)
{
	AutoMutex am(_mutex);
	_firstImageReceived = true;

	_sourceImage = sourceImage;
}

void CDisplay::setMatchImage(uint32_t imageId, float score, bool isUnusual)
{
	AutoMutex am(_mutex);
	_firstMatchReceived = true;
	_matchedImageNeedsReloading = imageId != _matchedImageId;
	_matchedImageId = imageId;
	_matchedImageIsUnusual = isUnusual;

}

void CDisplay::DisplayThread(void* arg)
{
	CDisplay* display = static_cast<CDisplay*>(arg);
	display->displayThread();
}

void CDisplay::displayThread()
{
	uint32_t red = setColour(200, 0, 0);
	uint32_t green = setColour(0, 200, 0);
	uint32_t white = setColour(255, 255, 255);

	while (!_shutdownRequested)
	{
		while (SDL_PollEvent(&_event))
		{
			switch (_event.type)
			{
			case SDL_MOUSEMOTION:
				//printf("Mouse moved by %d,%d to (%d,%d)\n", event.motion.xrel, event.motion.yrel,	event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				//printf("Mouse button %d pressed at (%d,%d)\n", event.button.button, event.button.x, event.button.y);
				break;
			case SDL_KEYUP:
				if (_event.key.keysym.sym == SDLK_m)
				{
					_drawType = DRAW_MODEL;
				}
				else if (_event.key.keysym.sym == SDLK_h)
				{
					_drawType = DRAW_HEAT_MAP;
				}
				else if (_event.key.keysym.sym == SDLK_g)
				{
					_drawType = DRAW_HOG;
				}
				else if (_event.key.keysym.sym == SDLK_s)
				{
					_drawType = DRAW_SOBEL;
				}
				else
				{
					_drawType = DRAW_IMAGE;
				}
				break;
			case SDL_QUIT:
				_shutdownRequested = true;
			}
		}

		if (_drawType == DRAW_IMAGE)
		{
			if (_firstImageReceived && _firstMatchReceived)
			{
				//fetch match image
				AutoMutex am(_mutex);
				if (_matchedImageNeedsReloading)
				{
					_matchedImage = _imageStore->fetchImage(_matchedImageId);
					_matchedImageNeedsReloading = false;
				}

				bool lastImageWasUnusual = false;
				CAutoScreenLock asl(_screen);

				{
					AutoMutex am(_mutex);

					blit(_sourceImage, 0, 20);
					blit(_matchedImage, IMAGE_WIDTH, 20);

					lastImageWasUnusual = _matchedImageIsUnusual;
				}

				//block can be un mutexed
				if (lastImageWasUnusual)
				{
					drawRectangle(0, 0, 2 * IMAGE_WIDTH - 1, 19, red);
					drawRectangle(0, 20 + IMAGE_HEIGHT + 100, 2 * IMAGE_WIDTH - 1, 20 + IMAGE_HEIGHT + 100 + 19, red);
				}
				else
				{
					drawRectangle(0, 0, 2 * IMAGE_WIDTH - 1, 19, green);
					drawRectangle(0, 20 + IMAGE_HEIGHT + 100, 2 * IMAGE_WIDTH - 1, 20 + IMAGE_HEIGHT + 100 + 19, green);
				}

				drawRectangle(0, IMAGE_HEIGHT + 20, 2 * IMAGE_WIDTH - 1, IMAGE_HEIGHT + 120, white);

				drawScoreDistribution();
			}
			else
			{
				CAutoScreenLock asl(_screen);

				//draw loading circle animation
				int cy = _windowHeight / 2;
				int cx = _windowWidth / 2;
				int a;
				int x, y;
				for (y = cy - 50; y < cy + 50; y++)
				{
					for (x = cx - 50; x < cx + 50; x++)
					{
						if ((x - cx) * (x - cx) + (y - cy) * (y - cy) == 2500) // 50px diameter circle
						{
							a = (int) (atan2((float) y - cy, (float) x - cx) / (2 * M_PI) * 255);
							setPixel(x, y, setColour((a + _loadingCounter) % 256, (a + _loadingCounter) % 256, (a + _loadingCounter) % 256));
						}
					}
				}
				_loadingCounter++;
			}
		}
		else if (_drawType == DRAW_MODEL)
		{
			drawModel();
			blit(_sourceImage, 512, 0);
		}
		else if (_drawType == DRAW_HEAT_MAP || _drawType == DRAW_HOG)
		{
			drawHeatMap();
			blit(_sourceImage, 512, 0);
		}
		else if (_drawType == DRAW_SOBEL)
		{
			if (_firstImageReceived && _firstMatchReceived)
			{
				cv::Mat greyImage;
				cv::Mat smoothedImage;
				cv::Mat sobelXImage;
				cv::Mat sobelYImage;

				cv::Mat sobelXImageRGB;
				cv::Mat sobelYImageRGB;

				cv::cvtColor(_sourceImage, greyImage, CV_RGB2GRAY);

				cv::GaussianBlur(greyImage, greyImage, cv::Size(7, 7), 0.5, 0.5);

				cv::Sobel(greyImage, sobelXImage, CV_8U, 1, 0, 3);
				cv::Sobel(greyImage, sobelYImage, CV_8U, 0, 1, 3);

				cv::cvtColor(sobelXImage, sobelXImageRGB, CV_GRAY2RGB);
				cv::cvtColor(sobelYImage, sobelYImageRGB, CV_GRAY2RGB);

				blit(sobelXImageRGB, 0, 20);
				blit(sobelYImageRGB, IMAGE_WIDTH, 20);

			}
		}

		SDL_Flip(_screen);

		usleep(40 * 1000);

	}
}

void CDisplay::blit(cv::Mat image, int x, int y)
{
	if (image.channels() == 3)
	{
		for (int32_t a = 0; a < image.rows; a++)
		{
			for (int32_t b = 0; b < image.cols; b++)
			{
				uint8_t red = *(image.data + a * image.step.buf[0] + 3 * b);
				uint8_t green = *(image.data + a * image.step.buf[0] + 3 * b + 1);
				uint8_t blue = *(image.data + a * image.step.buf[0] + 3 * b + 2);
				uint32_t c = setColour(red, green, blue);
				setPixel(x + b, y + a, c);
			}
		}
	}
}

void CDisplay::setPixel(int x, int y, uint32_t pixel)
{
	uint8_t *target_pixel = (uint8_t *) _screen->pixels + y * _screen->pitch + x * 4;
	*(uint32_t *) target_pixel = pixel;
}
uint32_t CDisplay::setColour(uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t pixel = b + (g << 8) + (r << 16);
	return pixel;
}

void CDisplay::drawRectangle(int x1, int y1, int x2, int y2, uint32_t pixel)
{
	if (y1 > y2)
	{
		int temp = y2;
		y2 = y1;
		y1 = temp;
	}
	if (x1 > x2)
	{
		int temp = x2;
		x2 = x1;
		x1 = temp;
	}
	for (int y = y1; y < y2; y++)
	{
		for (int x = x1; x < x2; x++)
		{
			setPixel(x, y, pixel);
		}
	}
}

void CDisplay::drawStraightLine(int x1, int y1, int x2, int y2, uint32_t pixel)
{
	if (y1 > y2)
	{
		int temp = y2;
		y2 = y1;
		y1 = temp;
	}
	if (x1 > x2)
	{
		int temp = x2;
		x2 = x1;
		x1 = temp;
	}
	if (x1 == x2)
	{
		for (int y = y1; y < y2 + 1; y++)
		{
			setPixel(x1, y, pixel);
		}
	}
	else
	{
		for (int x = x1; x < x2; x++)
		{
			setPixel(x, y1, pixel);
		}
	}
}

void CDisplay::drawQuadrants(int level, int sqx, int sqy, uint32_t colour)
{
	//fill colour
	uint32_t line_colour = setColour(0, 0, 0);

	//edge squares along top and side
	int edge_squares = (1 << level);
	int sq_width = IMAGE_WIDTH / edge_squares;
	int sq_height = IMAGE_HEIGHT / edge_squares;

	int x1, x2, y1, y2;

	x1 = sqx * sq_width + 1;
	y1 = sqy * sq_height + 1;

	x2 = (sqx + 1) * sq_width - 1;
	y2 = (sqy + 1) * sq_height - 1;

	drawRectangle(x1, y1, x2, y2, colour);
	drawStraightLine(x1, y1, x2, y1, line_colour);
	drawStraightLine(x1, y1, x1, y2, line_colour);
	drawStraightLine(x1, y2, x2, y2, line_colour);
	drawStraightLine(x2, y1, x2, y2, line_colour);

}

void CDisplay::drawModel()
{
	uint32_t level;
	uint32_t edgesThisLevel;
	uint32_t sqy, sqx;

	//fill colour
	uint32_t colours[7];
	colours[0] = setColour(255, 255, 255);
	colours[1] = setColour(0, 0, 255);
	colours[2] = setColour(0, 200, 05);
	colours[3] = setColour(255, 255, 0);
	colours[4] = setColour(255, 128, 0);
	colours[5] = setColour(255, 0, 0);
	colours[6] = setColour(255, 0, 128);

	uint32_t black = setColour(0, 0, 0);
	drawRectangle(0, 0, IMAGE_WIDTH - 1, IMAGE_HEIGHT - 1, black);
	for (level = 0; level < 7; level++)
	{
		edgesThisLevel = 1 << level;

		for (sqy = 0; sqy < edgesThisLevel; sqy++)
		{
			for (sqx = 0; sqx < edgesThisLevel; sqx++)
			{
				//offsets for current level
				if (!_model->isHandledAtHigherLevel(level, sqx, sqy) && _model->isHandledAtThisLevel(level, sqx, sqy))
				{
					drawQuadrants(level, sqx, sqy, colours[level]);
				}
			}
		}
	}
}

void CDisplay::drawScoreDistribution()
{
	uint32_t index = 0;
	std::vector<float> scores;

	_scoresDistribution->getScoreDistribution(&index, &scores);

	int32_t width = std::min(scores.size(), _windowWidth);
	int32_t start = index - width;

	float min = std::numeric_limits<float>::max();
	float max = 0;

	for (uint32_t i = 0; i < (uint32_t) width; i++)
	{
		min = (scores[(i + start) % scores.size()] < min) ? scores[(i + start) % scores.size()] : min;
		max = (scores[(i + start) % scores.size()] > max) ? scores[(i + start) % scores.size()] : max;
	}

	if (max != 0.0f)
	{
		uint32_t l = 0;
		uint32_t blue = setColour(0, 0, 255);
		for (int32_t i = 0; i < width; i++)
		{
			uint32_t h = (uint32_t) (100 * ((scores[(i + start) % scores.size()] - min) / (max - min)));

			drawStraightLine(i, 632 - h, i, 632 - l, blue);
			l = h;
		}
	}
}

void CDisplay::drawHeatMap()
{
	uint32_t black = setColour(0, 0, 0);
	drawRectangle(0, 0, IMAGE_WIDTH - 1, IMAGE_HEIGHT - 1, black);

	if(_matchedImage.channels() != 3 || _sourceImage.channels() != 3) return;

	CHog bestMatchHog(HOG_CELL_SIZE, HOG_NUM_CELLS, _matchedImage, 0);
	CHog sourceimageHog(HOG_CELL_SIZE, HOG_NUM_CELLS, _sourceImage, 0);

	uint32_t numLevels = _model->getNumLevels();
	const uint32_t NUM_BINS = 8;

	for (uint32_t level = 0; level < numLevels; level++)
	{
		uint32_t edgeCellsThisLevel = 1 << level;
		for (uint32_t y = 0; y < edgeCellsThisLevel; y++)
		{
			for (uint32_t x = 0; x < edgeCellsThisLevel; x++)
			{
				if (!_model->isHandledAtHigherLevel(level, x, y) && _model->isHandledAtThisLevel(level, x, y))
				{
					uint32_t edgeCellsToMerge = (HOG_NUM_CELLS / edgeCellsThisLevel);

					float histogramA[NUM_BINS], histogramB[NUM_BINS];
					for (uint32_t z = 0; z < NUM_BINS; z++)
					{
						histogramA[z] = histogramB[z] = 0.0f;
					}

					for (uint32_t y2 = 0; y2 < edgeCellsToMerge; y2++)
					{
						for (uint32_t x2 = 0; x2 < edgeCellsToMerge; x2++)
						{
							uint32_t yIndex = y * edgeCellsToMerge + y2;
							uint32_t xIndex = x * edgeCellsToMerge + x2;

							const uint16_t* bestMatchHistogram = bestMatchHog.getHistogram(xIndex, yIndex);
							const uint16_t* sourceHistogram = sourceimageHog.getHistogram(xIndex, yIndex);

							for (uint32_t z = 0; z < NUM_BINS; z++)
							{
								histogramA[z] += (float)bestMatchHistogram[z];
								histogramB[z] += (float)sourceHistogram[z];
							}
						}
					}

					float score = 0;

					for (uint32_t z = 0; z < NUM_BINS; z++)
					{
						//printf("HistogramA[%u] = %f, HistogramB[%u] = %f\n",  z, histogramA[z], z, histogramB[z]);
						score += ((float) histogramA[z] / 65535.0f * (float) histogramB[z] / 65535.0f) / (float) (edgeCellsToMerge * edgeCellsToMerge * edgeCellsToMerge * edgeCellsToMerge);
						histogramB[z] /= (65535.0f) * (float) (edgeCellsToMerge * edgeCellsToMerge * edgeCellsToMerge * edgeCellsToMerge);

					}

					uint32_t colour = setColour(255*score,255-(255*score),0);

					if(_drawType == DRAW_HEAT_MAP)
					{
						drawQuadrants(level, x, y, colour);
					}
					else
					{
						drawHog(level, x, y, histogramB);
					}

				}
			}
		}

	}

}

void CDisplay::drawHog(int level, int sqx, int sqy, const float* hog)
{
	//edge squares along top and side
	int edge_squares = (1 << level);
	int sq_width = IMAGE_WIDTH / edge_squares;
	int sq_height = IMAGE_HEIGHT / edge_squares;

	int x1, y1, y2;

	x1 = sqx * sq_width + 1;
	y1 = sqy * sq_height + 1;

	y2 = (sqy + 1) * sq_height - 1;

	int dx = sq_width / 8;

	for(int z=0;z<8;z++)
	{
		//printf("level = %u drawRectangle(%3d, %3d, %3d, %3d, setColour(%f,%f,0))\n", level, x1+z*dx, y1, x1+(z+1)*dx - 1, y2, 255*hog[z],255-(255*hog[z]));
		drawRectangle(x1+z*dx, y1, x1+(z+1)*dx, y2, setColour(255*hog[z],255-(255*hog[z]),0));
	}

}
