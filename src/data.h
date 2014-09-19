#ifndef __DATA_H__
#define __DATA_H__

#include <stdio.h>

struct image_data
{
	int hog_id;
	int match_count;
	int most_similar;
	float similarity_score;
	int added;
	int active;
};

struct format {
	unsigned int l[45];
	static const int length = 45;
};

void write_image_data_struct(image_data* d, FILE** fh);

void read_image_data_struct(image_data* d, FILE** fh);

void load_hogs_to_memory(float* hogs, FILE** fh_hog, int limit, int hog_size);

#endif

