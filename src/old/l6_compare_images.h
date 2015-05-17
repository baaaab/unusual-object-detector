#ifndef __L6_COMPARE_IMAGES_H__
#define __L6_COMPARE_IMAGES_H__

struct input_arguments
{
	char* prog_count_file;
	char* model_file;
	char* image_data_file;
	int image_count;
	int distribution_width;
	char* hog_file;
	char* image_dir;
	char* webcam_url;
};

void* main_loop(void* arg);
void sigHandler(int signo);

#endif

