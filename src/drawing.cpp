#include "drawing.h"

int width,height;

void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
	*(Uint32 *)target_pixel = pixel;
}
uint32_t set_colour(uint8_t r, uint8_t g, uint8_t b)
{
	Uint32 pixel;
	pixel = b + (g << 8) + (r << 16);
	return pixel;
}
void draw_rectangle(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 pixel)
{
	int x,y,temp;
	if(y1 > y2)
	{
		temp = y2;
		y2 = y1;
		y1 = temp;
	}
	if(x1 > x2)
	{
		temp = x2;
		x2 = x1;
		x1 = temp;
	}
	for(y=y1;y<y2;y++)
	{
		for(x=x1;x<x2;x++)
		{
			set_pixel(surface, x, y, pixel);
		}
	}
}
void draw_straight_line(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 pixel)
{
	int temp;
	if(y1 > y2)
	{
		temp = y2;
		y2 = y1;
		y1 = temp;
	}
	if(x1 > x2)
	{
		temp = x2;
		x2 = x1;
		x1 = temp;
	}
	if(x1==x2)
	{
		int y;
		for(y=y1;y<y2+1;y++)
		{
			set_pixel(surface, x1, y, pixel);
		}
	}
	else
	{
		int x;
		for(x=x1;x<x2;x++)
		{
			set_pixel(surface, x, y1, pixel);
		}
	}
}
void Slock(SDL_Surface *screen)
{
	if ( SDL_MUSTLOCK(screen) )
	{
		if ( SDL_LockSurface(screen) < 0 )
		{
			return;
		}
	}
}

void Sulock(SDL_Surface *screen)
{
	if ( SDL_MUSTLOCK(screen) )
	{
		SDL_UnlockSurface(screen);
	}
}
void draw_quadrants(SDL_Surface* surface, int level, int sqx, int sqy)
{
	//fill colour
	Uint32 colour;
	Uint32 line_colour = set_colour(0,0,0);
	switch(level)
	{
	case 0:
		colour = set_colour(255, 255, 255);
		break;
	case 1:
		colour = set_colour(0, 0, 255);
		break;
	case 2:
		colour = set_colour(0, 200, 05);
		break;
	case 3:
		colour = set_colour(255, 255, 0);
		break;
	case 4:
		colour = set_colour(255, 128, 0);
		break;
	case 5:
		colour = set_colour(255, 0, 0);
		break;
	case 6:
		colour = set_colour(255, 0, 128);
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
	int sq_width = width / edge_squares;
	int sq_height = height / edge_squares;

	int x1,x2,y1,y2;

	x1 = sqx * sq_width+1;
	y1 = sqy * sq_height+1;

	x2 = (sqx+1) * sq_width-1;
	y2 = (sqy+1) * sq_height-1;

	draw_rectangle(surface, x1, y1, x2, y2, colour);
	draw_straight_line(surface, x1, y1, x2, y1, line_colour);
	draw_straight_line(surface, x1, y1, x1, y2, line_colour);
	draw_straight_line(surface, x1, y2, x2, y2, line_colour);
	draw_straight_line(surface, x2, y1, x2, y2, line_colour);
	
}

void draw_format(SDL_Surface* surface, format* f)
{
	int curr_level;
	int edge_this_level;
	int edge_last_level;
	int array_offset;
	int bit_offset;
	int last_array_offset;
	int last_bit_offset;
	int sqy,sqx;
	int offsets[] = {0,1,2,3,5,13};

	float score = 0;

	uint32_t black = set_colour(0,0,0);
	//Slock(surface);
	draw_rectangle(surface, 0,0, width-1, height-1, black);
	//Sulock(surface);
	//SDL_Flip(surface);
	for(curr_level=0;curr_level<7;curr_level++)
	{
		edge_this_level = 1 << curr_level;
		edge_last_level = edge_this_level >> 1;

		for(sqy=0;sqy<edge_this_level;sqy++)
		{
			for(sqx=0;sqx<edge_this_level;sqx++)
			{
				//offsets for current level
				array_offset = offsets[curr_level] + ((sqy * edge_this_level) / 32);
				bit_offset = (sqy * edge_this_level + sqx) % 32;

				//offsets for last level
				last_array_offset = offsets[curr_level-1] + (((sqy>>1) * edge_last_level) / 32);
				last_bit_offset = ((sqy>>1)*edge_last_level + (sqx>>1))% 32;

				if(
					(
					curr_level == 6 || (f->l[array_offset] & (1<<bit_offset)) == 0
					)
					&& 
					(
					curr_level == 0 || (f->l[last_array_offset] & (1 << last_bit_offset))
					)
					) 
				{
					//Slock(surface);
					draw_quadrants(surface, curr_level, sqx, sqy);
					//Sulock(surface);
					//SDL_Flip(surface);
				}
			}
		}
	}
}

void copy_image_to_screen(SDL_Surface* screen, unsigned char* image, int width, int height, int x, int y, int bpp)
{
	int a,b;
	Uint32 c;
	if(bpp == 3)
	{
		for(a=0;a<height;a++)
		{
			for(b=0;b<width;b++)
			{
				c = set_colour(*(image+3*(a*width+b)), *(image+3*(a*width+b)+1), *(image+3*(a*width+b)+2));
				set_pixel(screen, x+b, y+a, c);
			}
		}
	}
	else
	{
		for(a=0;a<height;a++)
		{
			for(b=0;b<width;b++)
			{
				c = set_colour(*(image+(a*width+b)), *(image+(a*width+b)), *(image+(a*width+b)));
				set_pixel(screen, x+b, y+a, c);
			}
		}
	}
}
