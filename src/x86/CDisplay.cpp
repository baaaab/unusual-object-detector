#include "CDisplay.h"

#include <stdio.h>
#include <X11/Xlib.h>
#include <mutex>
#include <math.h>
#include <unistd.h>

#include "../utils/CAutoScreenLock.h"
#include "CModel.h"
#include "CImageStore.h"

namespace
{
const uint32_t IMAGE_WIDTH = 512;
const uint32_t IMAGE_HEIGHT = 512;

typedef std::lock_guard<std::recursive_mutex> AutoMutex;
}

CDisplay::CDisplay(CImageStore* imageStore) :
		_screen( NULL),
		_windowWidth(IMAGE_WIDTH * 2),
		_windowHeight(IMAGE_HEIGHT + 140),
		_jpegHandler( NULL),
		_imageStore(imageStore),
		_thread( NULL),
		_matchedImageId(0xffffffff),
		_matchedImageScore(0.0f),
		_matchedImageIsUnusual(false),
		_matchedImageNeedsReloading(true),
		_firstImageReceived(false),
		_firstMatchReceived(false),
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
		exit(1);
	}
	SDL_WM_SetCaption("Unusual Object Detector", "Unusual Object Detector");

	_jpegHandler = new CTurboJpegHandler();

	_thread = new std::thread(DisplayThread, this);
}

CDisplay::~CDisplay()
{
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
	uint32_t blue = setColour(0, 0, 255);

	while (1)
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
				exit(0);
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
				blit(_sourceImage.data, IMAGE_WIDTH, IMAGE_HEIGHT, 0, 20, 3);
				blit(_matchedImage.data, IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_WIDTH, 20, 3);
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

			/*int i;
			 int h;
			 float min = 9999999999.0f, max = 0;
			 for (i = 0; i < 1024; i++)
			 {
			 min = (dp->scores[i] < min) ? dp->scores[i] : min;
			 max = (dp->scores[i] > max) ? dp->scores[i] : max;
			 }
			 int start = *(dp->program_counter) % 1024;
			 int l;
			 if (max != 0.0f)
			 {
			 for (i = 0; i < 1024; i++)
			 {
			 h = (int) (100 * ((dp->scores[(i + start) % 1024] - min) / (max - min)));
			 //set_pixel(dp->screen, i, 632 - h, blue);
			 if (i != 0)
			 {
			 draw_straight_line(dp->screen, i, 632 - h, i, 632 - l, blue);
			 }
			 l = h;
			 }
			 }*/
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

void CDisplay::blit(unsigned char* image, int width, int height, int x, int y, int bpp)
{
	if (bpp == 3)
	{
		for (int32_t a = 0; a < height; a++)
		{
			for (int32_t b = 0; b < width; b++)
			{
				uint32_t c = setColour(*(image + 3 * (a * width + b) + 2), *(image + 3 * (a * width + b) + 1), *(image + 3 * (a * width + b)));
				setPixel(x + b, y + a, c);
			}
		}
	}
	else
	{
		for (int32_t a = 0; a < height; a++)
		{
			for (int32_t b = 0; b < width; b++)
			{
				uint32_t c = setColour(*(image + (a * width + b)), *(image + (a * width + b)), *(image + (a * width + b)));
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
	int edge_last_level;
	int array_offset;
	int bit_offset;
	int last_array_offset;
	int last_bit_offset;
	int sqy, sqx;
	int offsets[] =
	{ 0, 1, 2, 3, 5, 13 };

	float score = 0;

	uint32_t black = setColour(0, 0, 0);
	drawRectangle(0, 0, IMAGE_WIDTH - 1, IMAGE_HEIGHT - 1, black);
	for (curr_level = 0; curr_level < 7; curr_level++)
	{
		edge_this_level = 1 << curr_level;
		edge_last_level = edge_this_level >> 1;

		for (sqy = 0; sqy < edge_this_level; sqy++)
		{
			for (sqx = 0; sqx < edge_this_level; sqx++)
			{
				//offsets for current level
				array_offset = offsets[curr_level] + ((sqy * edge_this_level) / 32);
				bit_offset = (sqy * edge_this_level + sqx) % 32;

				if (f->handleAtThisLevel(curr_level, sqx, sqy))
				{
					drawQuadrants(curr_level, sqx, sqy);
				}
			}
		}
	}
}

