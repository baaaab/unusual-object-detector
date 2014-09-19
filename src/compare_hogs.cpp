#include "compare_hogs.h"

float compare_two_histograms(float* hog1, float* hog2, format* f)
{
	int curr_level;
	int edge_this_level;
	int edge_last_level;
	int array_offset;
	int bit_offset;
	int last_array_offset;
	int last_bit_offset;
	int sqy,sqx;
	int offsets[] = {0,1,2,3,5,13};

	float score = 0, this_score;

	for(curr_level=0;curr_level<7;curr_level++)
	{
		edge_this_level = 1 << curr_level;
		edge_this_level = 1 << curr_level;
		edge_last_level = edge_this_level >> 1;

		for(sqy=0;sqy<edge_this_level;sqy++)
		{
			for(sqx=0;sqx<edge_this_level;sqx++)
			{
				//offsets for current level
				array_offset = offsets[curr_level] + ((sqy * edge_this_level) / 32);
				bit_offset = (sqy * edge_this_level + sqx) % 32;

				//offsets for last level
				last_array_offset = offsets[curr_level-1] + ((sqy * edge_last_level	) / 32);
				last_bit_offset = ((sqy>>1)*edge_last_level + (sqx>>1))% 32;

				if(
					(
					curr_level == 6 || (f->l[array_offset] & (1<<bit_offset)) == 0
					)
					&& 
					(
					curr_level == 0 || (f->l[last_array_offset] & (1 << last_bit_offset))
					)
					) 
				{
					this_score = compute_similarity_between_two_quadrants_in_the_specified_images(hog1, hog2, curr_level, sqx, sqy);
					score += this_score;
				}
			}
		}
	}
	return score;	
}

float compute_similarity_between_two_quadrants_in_the_specified_images(float* hog1, float* hog2, int level, int sqx, int sqy)
{
	//step one is to combine the histograms in each quadrant (must not be normalised!)
	int edge_squares = 1 << level;
	int quadrant_edge = 64 / edge_squares;
	int x,y,z;
	float new_hog1[8],new_hog2[8];
	int squares_in_quadrant = quadrant_edge * quadrant_edge;
	int quardant_row = squares_in_quadrant * edge_squares;
	int hogs_in_image = 64 * 64;
	float score = 0;
	for(z=0;z<8;z++)
	{
		new_hog1[z] = 0.0f;
		new_hog2[z] = 0.0f;
	}
	for(y=0;y<quadrant_edge;y++)
	{
		for(x=0;x<quadrant_edge;x++)
		{
			for(z=0;z<8;z++)
			{
				new_hog1[z] += *(hog1 + 8 * (sqy * quardant_row + sqx * squares_in_quadrant + quadrant_edge * y + x) + z);
				new_hog2[z] += *(hog2 + 8 * (sqy * quardant_row + sqx * squares_in_quadrant + quadrant_edge * y + x) + z);
			}
		}
	}
	float s1=0,s2=0;
	for(z=0;z<8;z++)
	{
		s1 += new_hog1[z];
		s2 += new_hog2[z];
		//s1 = max(s1, new_hog1[z]);
		//s2 = max(s2, new_hog1[z]);
	}
	s1 = (s1==0)?1:s1;
	s2 = (s2==0)?1:s2;
	for(z=0;z<8;z++)
	{
		new_hog1[z]/=s1;
		new_hog2[z]/=s2;
	}
	for(z=0;z<8;z++)
	{
		score += new_hog1[z]*new_hog2[z];
	}
	score = score * squares_in_quadrant;
	return score;
}

void mirror_score_table(float* score_table, int image_count)
{
	int x,y;
	for(x=1;x<image_count;x++)
	{
		for(y=0;y<x;y++)
		{
			score_table[x * image_count + y] = score_table[y * image_count + x];
		}
	}
}

float find_highest_score(float* score_table, int x, int* best_match, int image_count)
{
	int y;
	float score = 0, this_score;
	for(y=0;y<image_count;y++)
	{
		if(x==y)continue;
		this_score = score_table[x * image_count + y];
		if(this_score > score)
		{
			score = this_score;
			*best_match = y;
		}
	}
	return score;
}
