#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <SDL/SDL.h>

struct display_parameters
{
	unsigned char* left_image;
	unsigned char* right_image;
	float *score;
	float *palarm;
	float* scores;
	SDL_Surface* screen;
	int* program_counter;
	bool enable;
	unsigned int c;
};

void* display(void* rsaj);

#endif

