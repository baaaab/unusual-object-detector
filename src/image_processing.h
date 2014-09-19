#ifndef __IMAGE_PROCESSING_H__
#define __IMAGE_PROCESSING_H__

void greyscale(unsigned char* src, unsigned char* dest, int width, int height);

void sobel_x(unsigned char* src, short* dest, int width, int height);
void sobel_y(unsigned char* src, short* dest, int width, int height);
void gaussian_filter(unsigned char* src, unsigned char* dest, int width, int height);

void generate_hog(float* hog, short* im_sobel_x, short* im_sobel_y, int width, int height);

void resize_image_from_640x480_to_512x512(unsigned char* im_in, unsigned char* im_out);

#endif

