#ifndef __DRAWING_H__
#define __DRAWING_H__

#include <inttypes.h>
#include <SDL/SDL.h>

#include "data.h"

void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
uint32_t set_colour(uint8_t r, uint8_t g, uint8_t b);
void draw_rectangle(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 pixel);
void draw_straight_line(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 pixel);
void Slock(SDL_Surface *screen);
void Sulock(SDL_Surface *screen);
void draw_quadrants(SDL_Surface* surface, int level, int sqx, int sqy);
void draw_format(SDL_Surface* surface, format* f);
void copy_image_to_screen(SDL_Surface* screen, unsigned char* image, int width, int height, int x, int y, int bpp);

#endif

