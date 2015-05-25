#include "CDisplay.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <cmath>
#include <unistd.h>
#include <limits>
#include <algorithm>

#include "../core/CThread.h"
#include "CAutoScreenLock.h"
#include "../core/CModel.h"
#include "../core/CImageStore.h"
#include <IJpegHandler.h>
#include "CJpegHandlerFactory.h"
#include "../core/CScoreDistribution.h"

namespace
{
const uint32_t IMAGE_WIDTH = 512;
const uint32_t IMAGE_HEIGHT = 512;

typedef std::lock_guard<std::recursive_mutex> AutoMutex;
}

CDisplay::CDisplay(CImageStore* imageStore, CScoreDistribution* scoresDistribution) :
		_shutdownRequested( false ),
		_screen( NULL),
		_windowWidth(IMAGE_WIDTH * 2),
		_windowHeight(IMAGE_HEIGHT + 140),
		_jpegHandler( NULL),
		_imageStore(imageStore),
		_scoresDistribution( scoresDistribution ),
		_matchedImageId(0xffffffff),
		_matchedImageScore(0.0f),
		_matchedImageIsUnusual(false),
		_matchedImageNeedsReloading(true),
		_firstImageReceived(false),
		_firstMatchReceived(false),
		_thread( NULL ),
		_loadingCounter(0)
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
	_matchedImageScore = score;
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
			case SDL_QUIT:
				_shutdownRequested = true;
			}
		}

		if (_firstImageReceived && _firstMatchReceived)
		{
			//fetch match image
			if(_matchedImageNeedsReloading)
			{
				_matchedImage = _imageStore->fetchImage(_matchedImageId);
				AutoMutex am(_mutex);
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
				uint8_t red = *(image.data + a * image.step.buf[0] + 3*b + 2);
				uint8_t green = *(image.data + a * image.step.buf[0] + 3*b + 1);
				uint8_t blue = *(image.data + a * image.step.buf[0] + 3*b);
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

void CDisplay::drawQuadrants(int level, int sqx, int sqy)
{
	//fill colour
	uint32_t colour;
	uint32_t line_colour = setColour(0, 0, 0);
	switch (level)
	{
	case 0:
		colour = setColour(255, 255, 255);
		break;
	case 1:
		colour = setColour(0, 0, 255);
		break;
	case 2:
		colour = setColour(0, 200, 05);
		break;
	case 3:
		colour = setColour(255, 255, 0);
		break;
	case 4:
		colour = setColour(255, 128, 0);
		break;
	case 5:
		colour = setColour(255, 0, 0);
		break;
	case 6:
		colour = setColour(255, 0, 128);
		break;
	}
	/*
	 level 0 = 1  = 2^0
	 level 1 = 4  = 2^2
	 level 2 = 16 = 2^4
	 level 3 = 64
	 level 4 = 256

	 */
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

void CDisplay::drawModel(CModel* f)
{
	int curr_level;
	int edge_this_level;
	int sqy, sqx;

	uint32_t black = setColour(0, 0, 0);
	drawRectangle(0, 0, IMAGE_WIDTH - 1, IMAGE_HEIGHT - 1, black);
	for (curr_level = 0; curr_level < 7; curr_level++)
	{
		edge_this_level = 1 << curr_level;

		for (sqy = 0; sqy < edge_this_level; sqy++)
		{
			for (sqx = 0; sqx < edge_this_level; sqx++)
			{
				//offsets for current level
				if (f->handleAtThisLevel(curr_level, sqx, sqy))
				{
					drawQuadrants(curr_level, sqx, sqy);
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

	float min = std::numeric_limits<float>::max();
	float max = 0;

	for (uint32_t i = 0; i < width; i++)
	{
		min = (scores[i] < min) ? scores[i] : min;
		max = (scores[i] > max) ? scores[i] : max;
	}

	int32_t start = index - width;
	if (max != 0.0f)
	{
		uint32_t l = 0;
		uint32_t blue = setColour(0,0, 255);
		for (int32_t i = 0; i < width; i++)
		{
			uint32_t h = (uint32_t) (100 * ((scores[(i + start) % scores.size()] - min) / (max - min)));

			drawStraightLine(i, 632 - h, i, 632 - l, blue);
			l = h;
		}
	}
}

