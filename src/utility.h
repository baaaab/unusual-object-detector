#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>

#include "data.h"

struct image_buffer
{
	char* buffer;
	int offset;
	int q;
};
size_t curl_jpeg_handler( void *ptr, size_t size, size_t nmemb, void *userdata);

int read_jpeg_file(const char *filename, unsigned char* image);

void write_JPEG_file (JSAMPLE * image_buffer, int image_width, int image_height, char * filename, int quality);

int update_image_model(char* filename, float* new_hog, float similarity_score, int best_match, FILE** fh_hog, float* hogs, image_data* img_data, int* rank, FILE** fh_image_data, int image_count, int* distribution, int program_counter, int distribution_width);

#endif
