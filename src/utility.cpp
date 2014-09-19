#include "utility.h"

#include <time.h>
#include <string.h>

#include "merge_sort.h"

#define UPDATE_INTERVAL 20

size_t curl_jpeg_handler( void *ptr, size_t size, size_t nmemb, void *userdata)
{
	FILE* f = (FILE*)userdata;
	fwrite(ptr, size, nmemb, f);
	return nmemb;
}

int read_jpeg_file(const char *filename, unsigned char* image)
{
	/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* libjpeg data structure for storing one row, that is, scanline of an image */
	JSAMPROW row_pointer[1];
	
	FILE *infile = fopen(filename, "rb");
	unsigned long location = 0;
	int i = 0;
	
	if ( !infile )
	{
		printf("Error opening jpeg file %s\n!", filename );
		return 0;
	}
	/* here we set up the standard libjpeg error handler */
	cinfo.err = jpeg_std_error( &jerr );
	/* setup decompression process and source, then read JPEG header */
	jpeg_create_decompress( &cinfo );
	/* this makes the library read from infile */
	jpeg_stdio_src( &cinfo, infile );
	/* reading the image header which contains image information */
	jpeg_read_header( &cinfo, TRUE );
	/* Uncomment the following to output image information, if needed. */
	/*--
	printf( "JPEG File Information: \n" );
	printf( "Image width and height: %d pixels and %d pixels.\n", cinfo.image_width, cinfo.image_height );
	printf( "Color components per pixel: %d.\n", cinfo.num_components );
	printf( "Color space: %d.\n", cinfo.jpeg_color_space );
	--*/
	/* Start decompression jpeg here */
	jpeg_start_decompress( &cinfo );

	/* allocate memory to hold the uncompressed image */
	//image = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
	/* now actually read the jpeg into the raw buffer */
	row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );
	/* read one scan line at a time */
	while( cinfo.output_scanline < cinfo.image_height )
	{
		jpeg_read_scanlines( &cinfo, row_pointer, 1 );
		for( i=0; i<(int)cinfo.image_width*cinfo.num_components;i++) 
			image[location++] = row_pointer[0][i];
	}
	/* wrap up decompression, destroy objects, free pointers and close open files */
	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );
	free( row_pointer[0] );
	fclose( infile );
	/* yup, we succeeded! */
	return 1;
}

void write_JPEG_file (JSAMPLE * image_buffer, int image_width, int image_height, char * filename, int quality)
{

	struct jpeg_compress_struct cinfo;

	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_compress(&cinfo);

	FILE * outfile = fopen(filename, "wb");
	if (outfile == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		exit(1000);
	}
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = image_width; 	/* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */

	jpeg_set_defaults(&cinfo);

	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&cinfo, TRUE);

	row_stride = image_width * cinfo.input_components;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	fclose(outfile);
	jpeg_destroy_compress(&cinfo);

}

int update_image_model(char* filename, float* new_hog, float similarity_score, int best_match, FILE** fh_hog, float* hogs, image_data* img_data, int* rank, FILE** fh_image_data, int image_count, int* distribution, int program_counter, int distribution_width)
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

}
