#include "utility.h"

#include <time.h>
#include <string.h>

#include <opencv2/core/core.hpp>

#define UPDATE_INTERVAL 20

/*int update_image_model(char* filename, float* new_hog, float similarity_score, int best_match, FILE** fh_hog, float* hogs, image_data* img_data, int* rank, FILE** fh_image_data, int image_count, int* distribution, int program_counter, int distribution_width)
 {
 int i;	
 //step 1: write image file to disk ==========================================
 
 //done elsewhere

 //step 2: find id of image to replace ========================================
 //int image_id = rank[image_count-1];
 srand(time(NULL));
 int image_id;
 float hit_rate = 9999999, hrt;
 //find image with lowest (hits / age ratio)
 for(i=0;i<image_count;i++)
 {
 hrt = (img_data[i].match_count+1) / (float)(program_counter - img_data[i].added + 1);
 if(hrt < hit_rate)
 {
 hit_rate = hrt;
 image_id = i;
 }
 }
 
 //replace attributes
 img_data[image_id].added = program_counter;
 img_data[image_id].active = 1;

 img_data[image_id].match_count = 0;
 img_data[image_id].most_similar = best_match;
 img_data[image_id].similarity_score = similarity_score;	

 if(best_match != -1)
 {
 img_data[best_match].match_count++;
 if(img_data[best_match].similarity_score < similarity_score)
 {
 img_data[best_match].similarity_score = similarity_score;
 img_data[best_match].most_similar = best_match;
 }
 }

 //step 3: replace HOG ==================================================
 int hog_size = 64*64*8;
 long offset = hog_size * image_id * sizeof(float);
 memcpy(hogs+hog_size*image_id, new_hog, hog_size*sizeof(float));
 //FILE* fh;

 //fopen_s(&fh, hog_file, "r+b");
 fseek(*fh_hog, offset, SEEK_SET);
 fwrite(new_hog, sizeof(float), hog_size, *fh_hog);
 //fclose(fh);
 

 //setp 4: reorder rank ==========================================
 int* temp = (int*)malloc(image_count * sizeof(int));
 mergeSort(img_data, rank, temp, image_count);
 free(temp);

 //step 5: rebuild distribution ===============================
 int bin;
 memset(distribution, 0, distribution_width);
 for(i=0;i<image_count;i++)
 {
 bin = (int)(distribution_width * (img_data[i].similarity_score / 4096));
 distribution[bin]++;
 }
 
 //step 5: write data file to disk ========================
 if(program_counter % UPDATE_INTERVAL == 0)
 {
 //fopen_s(&fh, image_data_file, "wb");
 fseek(*fh_image_data, 0, SEEK_SET);
 for(i=0;i<image_count;i++)
 {
 write_image_data_struct(&img_data[i], fh_image_data);
 }
 //fclose(fh);
 }
 return image_id;

 }*/
