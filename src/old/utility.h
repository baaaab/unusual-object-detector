#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>

struct image_buffer
{
	char* buffer;
	int offset;
	int q;
};

//int update_image_model(char* filename, float* new_hog, float similarity_score, int best_match, FILE** fh_hog, float* hogs, image_data* img_data, int* rank, FILE** fh_image_data, int image_count, int* distribution, int program_counter, int distribution_width);

#endif
