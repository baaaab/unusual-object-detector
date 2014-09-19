#include "distribution.h"

float find_inlier_scores(float prob, int* d, int image_count, int distribution_width)
{
	int i;
	int count = 0;
	int num = (int)(prob*image_count);
	for(i=0;i<distribution_width;i++)
	{
		count += d[i];
		if(count > num)
		{
			return (float)((i * 4096) / (float)distribution_width);
		}
	}
	return 4096.0f;
}
