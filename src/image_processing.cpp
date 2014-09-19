#include "image_processing.h"

#include <math.h>
#include <algorithm>
#include <stdio.h>


void greyscale(unsigned char* src, unsigned char* dest, int width, int height)
{
	int i;
	for(i=0;i<width*height;i++)
	{
		dest[i] = (src[3*i] + src[3*i+1] + src[3*i+2]) / 3;
	}
}

void sobel_x(unsigned char* src, short* dest, int width, int height)
{
	#define i(x,y) ((x) + width * (y))
	/*
	-1 0 1
	-2 0 2
	-1 0 1

	max:
	255 0 255
	255 0 255
	255 0 255

	510 + 1020 + 510 
	= (+/-)2040 (~2^ 11)
	+ 2048 (to make positive)
	~4096
	>> 4 to get back to 8 bits
	*/
	int x,y;
	short sum;
	for(y=1;y<height-1;y++)
	{
		for(x=1;x<width-1;x++)
		{
			sum = -1*src[i(x-1,y-1)] +   src[i(x+1,y-1)] +
				  -2*src[i(x-1,y )] + 2*src[i(x+1,y )] +
				  -1*src[i(x-1,y+1)] +   src[i(x+1,y+1)];
			dest[i(x,y)] = (sum);
		}
	}
	#undef i
}
void sobel_y(unsigned char* src, short* dest, int width, int height)
{
	#define i(x,y) ((x) + width * (y))
	
	int x,y;
	short sum;
	for(y=1;y<height-1;y++)
	{
		for(x=1;x<width-1;x++)
		{
			sum = -1*src[i(x-1,y-1)] + -2*src[i(x,y-1)] + -1*src[i(x+1,y-1)] +
				   1*src[i(x-1,y+1)] +  2*src[i(x,y+1)] +    src[i(x+1,y+1)];
			dest[i(x,y)] = (sum);
		}
	}
	#undef i
}
void gaussian_filter(unsigned char* src, unsigned char* dest, int width, int height)
{
	#define i(x,y) (std::min(width,std::max(0,(x))) + width * std::min(height, std::max(0,(y))))
	int x,y;
	//7x7 filter implementation
	float kernal[7] = 
	{
		(float)0.000817213,
		(float)0.028041521,
		(float)0.233926427,
		(float)0.474429677,
		(float)0.233926427,
		(float)0.028041521,
		(float)0.000817213,
	};
	for(y=0;y<height;y++)
	{	
		for(x=0;x<width;x++)
		{
			dest[i(x,y)] = (char)(
				src[i(x-3,y)]*kernal[0] +
				src[i(x-2,y)]*kernal[1] +
				src[i(x-1,y)]*kernal[2] +
				src[i(x  ,y)]*kernal[3] +
				src[i(x+1,y)]*kernal[4] +
				src[i(x+2,y)]*kernal[5] +
				src[i(x+3,y)]*kernal[6]
				);
		}
	}
	for(y=0;y<height;y++)
	{	
		for(x=0;x<width;x++)
		{
			dest[i(x,y)] = (char)(
				dest[i(x,y-3)]*kernal[0] +
				dest[i(x,y-2)]*kernal[1] +
				dest[i(x,y-1)]*kernal[2] +
				dest[i(x,y  )]*kernal[3] +
				dest[i(x,y+1)]*kernal[4] +
				dest[i(x,y+2)]*kernal[5] +
				dest[i(x,y+3)]*kernal[6]
			);
		}
	}

	#undef i
}

void generate_hog(float* hog, short* im_sobel_x, short* im_sobel_y, int width, int height)
{
	#define i(x,y) ((x) + width * (y))
	int hog_width = 8;
	int hog_height = 8;
	int sqx,sqy,x,y,px,py;
	float histogram[8],angle, magnitude,sum,PI = (float)3.14159265;
	int index;
	float a = 0;
	for(sqy=0;sqy<64;sqy++)
	{
		for(sqx=0;sqx<64;sqx++)
		{
			for(x=0;x<8;x++)
			{
				histogram[x] = 0.0f;
			}
			sum = 0;
			for(y=0;y<8;y++)
			{
				for(x=0;x<8;x++)
				{
					px = sqx*8+x;
					py = sqy*8+y;
					
					angle = atan2((float)im_sobel_y[i(px,py)], (float)im_sobel_x[i(px,py)]);
					
					magnitude = sqrt((float)((float)im_sobel_y[i(px,py)] * (float)im_sobel_y[i(px,py)]) + ((float)im_sobel_x[i(px,py)] * (float)im_sobel_x[i(px,py)]));
					sum += magnitude;

					index = (int)floor(4*(0.99999 + angle / (float)PI));

					histogram[index] += magnitude;
				}
			}
			if(sum == 0)
			{
				sum = 1;
			}
			for(x=0;x<8;x++)
			{
				hog[(sqy*64+sqx)*8+x] = histogram[x]/sum;
				if(sum == 0)
				{
					printf("error");
				}
				a+= histogram[x]/sum;
			}
		}
	}
#undef i
}

void resize_image_from_640x480_to_512x512(unsigned char* im_in, unsigned char* im_out)
{
	unsigned char* temp = (unsigned char*)malloc(512*480*3);
	int x,y;
	int x1,x2,y1,y2;
	float xf,yf;
	//x direction first(640x480 -> 512x480)
	for(y=0;y<480;y++)
	{
		for(x=0;x<512;x++)
		{
			xf = (float)x * ((float)480 / (float)512);
			x1 = (int)(floorf(xf));
			x2 = (int)(ceilf(xf));
			*(temp + 3*(y*512 + x)    ) = (unsigned char)( (( 1 - (xf-x1) ) * *(im_in + 3*(y*640 + 82 + x1)    )) + ((xf-x1) * *(im_in + 3*(y*640 + 82 + x2)    )) );
			*(temp + 3*(y*512 + x) + 1) = (unsigned char)( (( 1 - (xf-x1) ) * *(im_in + 3*(y*640 + 82 + x1) + 1)) + ((xf-x1) * *(im_in + 3*(y*640 + 82 + x2) + 1)) );
			*(temp + 3*(y*512 + x) + 2) = (unsigned char)( (( 1 - (xf-x1) ) * *(im_in + 3*(y*640 + 82 + x1) + 2)) + ((xf-x1) * *(im_in + 3*(y*640 + 82 + x2) + 2)) );
		}
	}
	for(y=0;y<512;y++)
	{
		for(x=0;x<512;x++)
		{
			yf = (float)y * ((float)480 / (float)512);
			y1 = (int)floorf(yf);
			y2 = (int)ceilf(yf);
			*(im_out + 3*(y*512 + x)    ) = (unsigned char)( (1-(yf-y1)) * *(temp + 3*(y1*512 + x)    ) + (yf-y1) * *(temp + 3*(y2*512 + x)    ) );
			*(im_out + 3*(y*512 + x) + 1) = (unsigned char)( (1-(yf-y1)) * *(temp + 3*(y1*512 + x) + 1) + (yf-y1) * *(temp + 3*(y2*512 + x) + 1) );
			*(im_out + 3*(y*512 + x) + 2) = (unsigned char)( (1-(yf-y1)) * *(temp + 3*(y1*512 + x) + 2) + (yf-y1) * *(temp + 3*(y2*512 + x) + 2) );
		}
	}
	free(temp);
}
