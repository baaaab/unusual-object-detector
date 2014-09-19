#ifndef __MERGE_SORT_H__
#define __MERGE_SORT_H__

#include "data.h"

void mergeSort(image_data* id, int numbers[], int temp[], int array_size);

void m_sort(image_data* id, int numbers[], int temp[], int left, int right);

void merge(image_data* id, int numbers[], int temp[], int left, int mid, int right);

#endif

