/*#include "l6_compare_images.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fstream>
#include <signal.h>
#include <unistd.h>

#define T_MATCH_DELAY 50

#include "data.h"
#include "drawing.h"
#include "merge_sort.h"
#include "utility.h"
#include "image_processing.h"
#include "compare_hogs.h"
#include "distribution.h"

#include "CHttpImageSource.h"
#include <ILiveResultManager.h>
#include "CDisplay.h"
#include "utils/CSettingsRegistry.h"

#include <opencv2/opencv.hpp>

int main(int argc, char* argv[])
{

	if (argc < 2)
	{
		printf("usage: %s program counter_file image_data_file distribution_width image_count hog_squares num_threads ", argv[0]);
		return (101);
	}

	//program counter
	char* prog_count_file = argv[1];
	char* image_data_file = argv[2];
	int image_count;
	if (sscanf(argv[4], "%d", &image_count) != 1 || image_count < 1)
	{
		printf("Error! image_count must be an integer and greater than 1!");
		return (3);
	}
	int distribution_width;
	if (sscanf(argv[3], "%d", &distribution_width) != 1 || distribution_width < 1)
	{
		printf("Error! image_count must be an integer and greater than 1!");
		return (4);
	}
	int hog_squares;
	if (sscanf(argv[5], "%d", &hog_squares) != 1 || hog_squares < 1)
	{
		printf("Error! hog_squares must be an integer and greater than 1!");
		return (5);
	}
	char* hog_file = argv[6];
	char* image_dir = argv[7];
	char* model_file = argv[8];
	char* webcam_url = argv[9];

	signal(SIGINT, sigHandler);

	int hog_size = 64 * 64 * 8;

	format f;
	FILE* fh_model = fopen(ia->model_file, "rb");
	fread(f.l, 4, f.length, fh_model);
	fclose(fh_model);

	//image_data
	image_data* img_data = (image_data*) malloc(image_count * sizeof(image_data));
	FILE* fh_image_data = fopen(ia->image_data_file, "r+b");
	if (fh_image_data == NULL)
	{
		exit(7);
	}

	//fill struct from file
	for (i = 0; i < image_count; i++)
	{
		read_image_data_struct(&img_data[i], &fh_image_data);
	}
	//fclose(fh);
	/////////////////////////////////////////////////////////////////////////////////
	/ *fopen_s(&fh, image_data_file, "wb");
	 for(i=0;i<image_count;i++)
	 {
	 write_image_data_struct(&img_data[i], &fh);
	 }
	 fclose(fh); * /

	//score distribution
	int bin;
	int distribution_width = ia->distribution_width;
	int* distribution = (int*) malloc(distribution_width * sizeof(int));
	memset(distribution, 0, distribution_width);
	for (i = 0; i < image_count; i++)
	{
		bin = (int) (distribution_width * (img_data[i].similarity_score / 4096));
		distribution[bin]++;
	}

	//ranking information for replacing images
	int* rank = (int*) malloc(image_count * sizeof(int));
	int* temp = (int*) malloc(image_count * sizeof(int));

	for (i = 0; i < image_count; i++)
	{
		rank[i] = i;
	}
	mergeSort(img_data, rank, temp, image_count);

	free(temp);

	//hog for new image
	float* new_hog = (float*) malloc(64 * 64 * 8 * sizeof(float));

	//image output
	char* dirname = (char*) malloc(500);
	char* filename = (char*) malloc(500);
	char* fullfilename = (char*) malloc(500);
	char* newfilename = (char*) malloc(500);

	//scores
	float* scores = (float*) malloc(sizeof(float) * 1024);
	for (i = 0; i < 1024; i++)
	{
		scores[i] = 0.0f;
	}

	float score, this_score;
	int best_match;

	float palarm;

	int image_id;

	FILE* new_file;

	FILE* fh_hog = fopen(ia->hog_file, "r+b");

	//hog store
	float* hogs = (float*) malloc(image_count * hog_size * sizeof(float));

	//load HOGs
	load_hogs_to_memory(hogs, &fh_hog, image_count, hog_size);

	cv::Mat sourceImageOriginalSize;
	cv::Mat sourceImageResized;
	cv::Mat grey;
	cv::Mat sobel_x;
	cv::Mat sobel_y;

	//main program loop
	while (1)
	{
		//make directory name
		snprintf(dirname, 500, "%s%d", ia->image_dir, program_counter / 1000);

		//1000 files per directory
		if (program_counter % 1000 == 0)
		{
			//create a new folder
			mkdir(dirname, 0775);
		}

		//file name
		snprintf(fullfilename, 500, "%s/%d.jpg", dirname, program_counter);
		snprintf(filename, 500, "%d/%d.jpg", program_counter / 1000, program_counter);

		IImageSource* imageSource = new CHttpImageSource(ia->webcam_url);
		CTurboJpegHandler* jpegHandler = new CTurboJpegHandler();

		sourceImageOriginalSize = imageSource->getImage();

		cv::resize(sourceImageOriginalSize, sourceImageResized, cv::Size(512, 512), 0, 0);

		std::ofstream jpgeOutStream(fullfilename);
		std::vector<unsigned char> compressedResizedImage = jpegHandler->compress(sourceImageResized);
		jpgeOutStream.write((char*) &compressedResizedImage[0], compressedResizedImage.size());
		jpgeOutStream.close();

		cv::cvtColor(sourceImageResized, grey, CV_BGR2GRAY);

		cv::GaussianBlur(grey, grey, cv::Size(7, 7), 0.5, 0.5);

		cv::Sobel(grey, sobel_x, CV_16S, 1, 0, 3);
		cv::Sobel(grey, sobel_y, CV_16S, 0, 1, 3);

		generate_hog(new_hog, (short*) sobel_x.data, (short*) sobel_y.data, 512, 512);
		score = 0;
		best_match = -1;

		for (i = 0; i < image_count; i++)
		{
			if (program_counter < T_MATCH_DELAY || program_counter - img_data[i].added > T_MATCH_DELAY)
			{
				this_score = compare_two_histograms(hogs + i * hog_size, new_hog, &f);
				if (this_score > score)
				{
					score = this_score;
					best_match = i;
				}
			}
		}
		if (best_match == -1 || img_data[best_match].active == 0)
		{
			printf("%d: Unable to match image (or invalid match), continuing\n", program_counter);
		}
		else
		{
			palarm = find_inlier_scores(0.05f, distribution, image_count, distribution_width);

			snprintf(newfilename, 500, "%s%d/%d.jpg", ia->image_dir, img_data[best_match].added / 1000, img_data[best_match].added);

			//read_jpeg_file(newfilename, best_match_image);

			scores[program_counter % 1024] = score;

			if (score < palarm)
			{
				//unusual image, move to image_dir/unusual/
				char mv_unusual_image_cmd[500];
				snprintf(mv_unusual_image_cmd, sizeof(mv_unusual_image_cmd), "mv temp.jpg %sunusual/%d.jpg", ia->image_dir, program_counter);
				system(mv_unusual_image_cmd);
			}

		}

		image_id = update_image_model(filename, new_hog, score, best_match, &fh_hog, hogs, img_data, rank, &fh_image_data, image_count, distribution, program_counter, distribution_width);
		if (best_match != -1)
		{
			printf("%5d Best match: %4d (%f / %f) matches: %4d added: %4d\n", program_counter, best_match, score, palarm, img_data[best_match].match_count, img_data[best_match].added);
		}

		program_counter++;

		fseek(fh_program_counter, 0, SEEK_SET);
		fwrite(&program_counter, 4, 1, fh_program_counter);

		//SDL_Delay(3000);
	}

	fclose(fh_hog);
	fclose(fh_image_data);
	fclose (fh_program_counter);
	return NULL;
}*/

