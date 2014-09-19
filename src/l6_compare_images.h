#ifndef __L6_COMPARE_IMAGES_H__
#define __L6_COMPARE_IMAGES_H__

#include <SDL/SDL.h>

struct input_arguments
{
	char* prog_count_file;
	char* model_file;
	char* image_data_file;
	int image_count;
	int distribution_width;
	char* hog_file;
	char* image_dir;
	char* webcam_url;
	SDL_Surface* screen;
};

void* main_loop(void* arg);

#endif

