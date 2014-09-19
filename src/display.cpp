#include "display.h"

#include <math.h>

#include "drawing.h"

void* display(void* rsaj)
{
	display_parameters* dp = (display_parameters*)rsaj;
	while(1)
	{
		if(dp->enable)
		{
			int height = 512 + 40 + 100;
			Uint32 red = set_colour(200, 0, 0);
			Uint32 green = set_colour(0, 200, 0);
			Uint32 white = set_colour(255,255,255);
			Uint32 blue = set_colour(0,0,255);

			Slock(dp->screen);
			copy_image_to_screen(dp->screen, dp->left_image, 512, 512, 0, 20, 3);
			copy_image_to_screen(dp->screen, dp->right_image, 512, 512, 512, 20, 3);

			if(*(dp->score) < *(dp->palarm))
			{
				draw_rectangle(dp->screen, 0,0, 1023, 19, red);
				draw_rectangle(dp->screen, 0,20+512+100, 1023, 20+512+100+19, red);
			}
			else
			{
				draw_rectangle(dp->screen, 0,0, 1023, 19, green);
				draw_rectangle(dp->screen, 0,20+512+100, 1023, 20+512+100+19, green);
			}

			draw_rectangle(dp->screen, 0, 532, 1023, 632, white);
			int i;
			int h;
			float min = 9999999999.0f,max = 0;
			for(i=0;i<1024;i++)
			{
				min = (dp->scores[i] < min)?dp->scores[i]:min;
				max = (dp->scores[i] > max)?dp->scores[i]:max;
			}
			int start = *(dp->program_counter)%1024;
			int l;
			if(max != 0.0f)
			{
				for(i=0;i<1024;i++)
				{
					h = (int)(100*((dp->scores[(i+start)%1024] - min) / (max - min)));
					//set_pixel(dp->screen, i, 632 - h, blue);
					if(i!=0)
					{
						draw_straight_line(dp->screen, i, 632 - h, i, 632 - l, blue);
					}
					l = h;
				}
			}
			Sulock(dp->screen);
			SDL_Flip(dp->screen);
			
		}
		else
		{
			Slock(dp->screen);

			int cy = 326;
			int cx = 512;
			int a;
			int x,y;
			for(y=cy-50;y<cy+50;y++)
			{
				for(x=cx-50;x<cx+50;x++)
				{
					if((x-cx)*(x-cx) + (y-cy)*(y-cy) == 2500)
					{
						a = (int)(atan2((float)y-cy, (float)x-cx) / (2 * M_PI) * 255);
						set_pixel(dp->screen, x, y, set_colour((a+dp->c)%256,(a+dp->c)%256,(a+dp->c)%256));
					}
				}
			}

			Sulock(dp->screen);
			SDL_Flip(dp->screen);
			dp->c--;
		}
		SDL_Delay(30);
	}
	return NULL;
}
