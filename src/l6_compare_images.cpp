#include "l6_compare_images.h"

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>

#define T_MATCH_DELAY 50

#include "data.h"
#include "drawing.h"
#include "merge_sort.h"
#include "utility.h"
#include "image_processing.h"
#include "compare_hogs.h"
#include "distribution.h"
#include "display.h"
#include <X11/Xlib.h>


int main(int argc, char* argv[])
{
	XInitThreads();
		//input stuff
	/*
		program status
		1 program counter file
		2 image_data file
		3 distribution_width
		4 image_count
		5 hog_squares
		6 hog_file
		7 image_dir
		8 model_file
		9 webcam feed
	*/
	if(argc < 10)
	{
		printf("usage: %s program counter_file image_data_file distribution_width image_count hog_squares num_threads ", argv[0]);
		return(101);
	}

	//program counter
	char* prog_count_file = argv[1];
	char* image_data_file = argv[2];
	int image_count;
	if(sscanf(argv[4], "%d", &image_count) != 1 || image_count < 1)
	{
		printf("Error! image_count must be an integer and greater than 1!");
		return (3);
	}
	int distribution_width;
	if(sscanf(argv[3], "%d", &distribution_width) != 1 || distribution_width < 1)
	{
		printf("Error! image_count must be an integer and greater than 1!");
		return (4);
	}
	int hog_squares;
	if(sscanf(argv[5], "%d", &hog_squares) != 1 || hog_squares < 1)
	{
		printf("Error! hog_squares must be an integer and greater than 1!");
		return (5);
	}
	char* hog_file = argv[6];
	char* image_dir = argv[7];
	char* model_file = argv[8];
	char* webcam_url = argv[9];

	//initialize SDL
	int width;
	int height;
	if (SDL_Init( SDL_INIT_VIDEO ) != 0)
	{
		fprintf( stderr, "Could not initialise SDL: %s\n", SDL_GetError() );
		return 8;
	}
	atexit(SDL_Quit);
	SDL_Event    event;
	SDL_Surface *screen;
	width = 64 * 8 * 2;
	height = 64 * 8 + 140;
	if ((screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE|SDL_DOUBLEBUF )) == NULL)
	{
		fprintf( stderr, "Could not set SDL video mode: %s\n", SDL_GetError() );
		SDL_Quit();
		return 9;
	}
	SDL_WM_SetCaption( "Unusual Object Detector!", "Unusual Object Detector!" );

	input_arguments ia;
	ia.distribution_width = distribution_width;
	ia.hog_file = hog_file;
	ia.image_count = image_count;
	ia.image_data_file = image_data_file;
	ia.image_dir = image_dir;
	ia.model_file = model_file;
	ia.prog_count_file = prog_count_file;
	ia.webcam_url = webcam_url;
	ia.screen = screen;

	pthread_t main_thread;
	pthread_create(&main_thread, NULL, main_loop, &ia);

	while(1)
	{
		SDL_WaitEvent(&event);
		while ( SDL_PollEvent(&event))
		{
			switch (event.type) {
				case SDL_MOUSEMOTION:
					//printf("Mouse moved by %d,%d to (%d,%d)\n", event.motion.xrel, event.motion.yrel,	event.motion.x, event.motion.y);
					break;
				case SDL_MOUSEBUTTONDOWN:
					//printf("Mouse button %d pressed at (%d,%d)\n", event.button.button, event.button.x, event.button.y);
					break;
				case SDL_QUIT:
					exit(0);
			}
		}
	}
}

void* main_loop(void* arg)
{
	input_arguments* ia = (input_arguments*)arg;
	int i;
	int program_counter;
	int image_count = ia->image_count;

	FILE* fh_program_counter = fopen(ia->prog_count_file, "r+b");
	if(fh_program_counter == NULL)
	{
		exit(2);
	}
	fread(&program_counter, 4, 1, fh_program_counter);

	int hog_size = 64 * 64 * 8;

	format f;
	FILE* fh_model = fopen(ia->model_file, "rb");
	fread(f.l, 4, f.length, fh_model);
	fclose(fh_model);

	//image_data
	image_data* img_data = (image_data*) malloc(image_count * sizeof(image_data));
	FILE* fh_image_data = fopen(ia->image_data_file, "r+b");
	if(fh_image_data == NULL)
	{
		exit(7);
	}


	//fill struct from file
	for(i=0;i<image_count;i++)
	{
		read_image_data_struct(&img_data[i], &fh_image_data);
	}
	//fclose(fh);
	/////////////////////////////////////////////////////////////////////////////////
	/*fopen_s(&fh, image_data_file, "wb");
	for(i=0;i<image_count;i++)
	{
		write_image_data_struct(&img_data[i], &fh);
	}
	fclose(fh);*/

	//score distribution
	int bin;
	int distribution_width = ia->distribution_width;
	int* distribution = (int*) malloc(distribution_width * sizeof(int));
	memset(distribution, 0, distribution_width);
	for(i=0;i<image_count;i++)
	{
		bin = (int)(distribution_width * (img_data[i].similarity_score / 4096));
		distribution[bin]++;
	}

	//ranking information for replacing images
	int* rank = (int*)malloc(image_count * sizeof(int));
	int* temp = (int*)malloc(image_count * sizeof(int));

	for(i=0;i<image_count;i++)
	{
		rank[i] = i;
	}
	mergeSort(img_data, rank, temp, image_count);

	free(temp);

	//init curl
	CURL *curl;
	CURLcode res;

	//hog for new image
	float* new_hog = (float*)malloc(64*64*8*sizeof(float));

	//image output
	unsigned char* new_image_raw = (unsigned char*)malloc(640*480*3);
	unsigned char* new_image = (unsigned char*)malloc(512*512*3);
	unsigned char* best_match_image = (unsigned char*)malloc(512*512*3);
	unsigned char* grey = (unsigned char*)malloc(512*512);
	unsigned char* smooth = (unsigned char*)malloc(512*512);
	short* im_sobel_x = (short*)malloc(512*512*sizeof(short));
	short* im_sobel_y = (short*)malloc(512*512*sizeof(short));
	memset(im_sobel_y, 0, 512*512);
	memset(im_sobel_x, 0, 512*512);

	char* dirname = (char*)malloc(500);
	char* filename = (char*)malloc(500);
	char* fullfilename = (char*)malloc(500);
	char* newfilename = (char*)malloc(500);

	//scores
	float* scores = (float*)malloc(sizeof(float) * 1024);
	for(i=0;i<1024;i++)
	{
		scores[i] = 0.0f;
	}

	float score,this_score;
	int best_match;

	float palarm;

	int image_id;

	FILE* new_file;

	display_parameters dp;
	dp.left_image = new_image;
	dp.right_image = best_match_image;
	dp.scores = scores;
	dp.program_counter = &program_counter;
	dp.score = &score;
	dp.palarm = &palarm;
	dp.screen = ia->screen;
	dp.enable = false;
	dp.c = INT_MAX;

	pthread_t display_thread;
	pthread_create(&display_thread, NULL, display, &dp);

	FILE* fh_hog = fopen(ia->hog_file, "r+b");

	//hog store
	float* hogs = (float*) malloc(image_count * hog_size * sizeof(float));

	//load HOGs
	load_hogs_to_memory(hogs, &fh_hog, image_count, hog_size);

	dp.enable = true;

	//main program loop
	while(1)
	{
		//make directory name
		snprintf(dirname, 500, "%s%d", ia->image_dir, program_counter/1000);

		//1000 files per directory
		if(program_counter % 1000 == 0)
		{
			//create a new folder
			mkdir(dirname, 0775);
		}

		//file name
		snprintf(fullfilename, 500, "%s/%d.jpg", dirname, program_counter);
		snprintf(filename, 500, "%d/%d.jpg", program_counter/1000, program_counter);

		new_file = fopen("temp.jpg", "wb");
		curl = curl_easy_init();
		if(!curl)
		{
			printf("Error starting curl");
			exit (63);
		}
		curl_easy_setopt(curl, CURLOPT_URL, ia->webcam_url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_jpeg_handler);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, new_file);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fclose(new_file);

		if(read_jpeg_file("temp.jpg", new_image_raw))
		{
			resize_image_from_640x480_to_512x512(new_image_raw, new_image);

			write_JPEG_file(new_image, 512, 512, fullfilename, 70);

			greyscale(new_image, grey, 512, 512);
			gaussian_filter(grey, smooth, 512, 512);
			gaussian_filter(smooth, grey, 512, 512);
			sobel_x(grey, im_sobel_x, 512, 512);
			sobel_y(grey, im_sobel_y, 512, 512);

			generate_hog(new_hog, im_sobel_x, im_sobel_y, 512, 512);
			score = 0;
			best_match = -1;


			for(i=0;i<image_count;i++)
			{
				if(program_counter < T_MATCH_DELAY || program_counter - img_data[i].added > T_MATCH_DELAY)
				{
					this_score = compare_two_histograms(hogs + i * hog_size, new_hog, &f);
					if(this_score > score)
					{
						score = this_score;
						best_match = i;
					}
				}
			}
			if(best_match == -1 || img_data[best_match].active == 0)
			{
				printf("%d: Unable to match image (or invalid match), continuing\n", program_counter);
			}
			else
			{
				palarm = find_inlier_scores(0.05f, distribution, image_count, distribution_width);

				snprintf(newfilename, 500, "%s%d/%d.jpg", ia->image_dir, img_data[best_match].added/1000, img_data[best_match].added);

				read_jpeg_file(newfilename, best_match_image);

				scores[program_counter%1024] = score;

				if(score < palarm)
				{
					//unusual image, move to image_dir/unusual/
					char mv_unusual_image_cmd[500];
					snprintf(mv_unusual_image_cmd, sizeof(mv_unusual_image_cmd), "mv temp.jpg %sunusual/%d.jpg", ia->image_dir, program_counter);
					system(mv_unusual_image_cmd);
				}

			}

			image_id = update_image_model(filename, new_hog, score, best_match, &fh_hog, hogs, img_data, rank, &fh_image_data, image_count, distribution, program_counter, distribution_width);
			if(best_match != -1)
			{
				printf("%5d Best match: %4d (%f / %f) matches: %4d added: %4d\n", program_counter, best_match, score, palarm, img_data[best_match].match_count, img_data[best_match].added);
			}

			program_counter++;

			fseek(fh_program_counter, 0, SEEK_SET);
			fwrite(&program_counter, 4, 1, fh_program_counter);


		}
		else
		{
			printf("Error reading image from stream, retrying...\n");
		}


		//SDL_Delay(3000);
	}

	fclose(fh_hog);
	fclose(fh_image_data);
	fclose(fh_program_counter);
	return NULL;
}


