#ifndef __COMPARE_HOGS_H__
#define __COMPARE_HOGS_H__

#include "data.h"

float compute_similarity_between_two_quadrants_in_the_specified_images(float* hog1, float* hog2, int level, int sqx, int sqy);

float compare_two_histograms(float* hog1, float* hog2, format* f);

void mirror_score_table(float* score_table, int image_count);

float find_highest_score(float* score_table, int x, int* best_match, int image_count);

#endif

