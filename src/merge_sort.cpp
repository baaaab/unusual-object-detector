#include "merge_sort.h"

void mergeSort(image_data* id, int numbers[], int temp[], int array_size)
{
	m_sort(id, numbers, temp, 0, array_size - 1);
}

void m_sort(image_data* id, int numbers[], int temp[], int left, int right)
{
	int mid;

	if (right > left)
	{
		mid = (right + left) / 2;
		m_sort(id, numbers, temp, left, mid);
		m_sort(id, numbers, temp, mid+1, right);

		merge(id, numbers, temp, left, mid+1, right);
	}
}

void merge(image_data* id, int numbers[], int temp[], int left, int mid, int right)
{
	int i, left_end, num_elements, tmp_pos;

	left_end = mid - 1;
	tmp_pos = left;
	num_elements = right - left + 1;

	while ((left <= left_end) && (mid <= right))
	{
		if (id[numbers[left]].similarity_score <= id[numbers[mid]].similarity_score)
		{
			temp[tmp_pos] = numbers[left];
			tmp_pos = tmp_pos + 1;
			left = left +1;
		}
		else
		{
			temp[tmp_pos] = numbers[mid];
			tmp_pos = tmp_pos + 1;
			mid = mid + 1;
		}
	}

	while (left <= left_end)
	{
		temp[tmp_pos] = numbers[left];
		left = left + 1;
		tmp_pos = tmp_pos + 1;
	}
	while (mid <= right)
	{
		temp[tmp_pos] = numbers[mid];
		mid = mid + 1;
		tmp_pos = tmp_pos + 1;
	}

	for (i=0; i <= num_elements; i++)
	{
		numbers[right] = temp[right];
		right = right - 1;
	}
}
