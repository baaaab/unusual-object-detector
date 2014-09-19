#include "data.h"

void write_image_data_struct(image_data* d, FILE** fh)
{
	//ints
	fwrite(&(d->hog_id), 4, 1, *fh);
	fwrite(&(d->match_count), 4, 1, *fh);
	fwrite(&(d->most_similar), 4, 1, *fh);
	fwrite(&(d->added), 4, 1, *fh);
	fwrite(&(d->active), 4, 1, *fh);

	//floats(s)
	fwrite(&d->similarity_score, 4, 1, *fh);

}
void read_image_data_struct(image_data* d, FILE** fh)
{
	//ints
	fread(&(d->hog_id), 4, 1, *fh);
	fread(&(d->match_count), 4, 1, *fh);
	fread(&(d->most_similar), 4, 1, *fh);
	fread(&(d->added), 4, 1, *fh);
	fread(&(d->active), 4, 1, *fh);

	//floats(s)
	fread(&d->similarity_score, 4, 1, *fh);

}

void load_hogs_to_memory(float* hogs, FILE** fh_hog, int limit, int hog_size)
{
	size_t i;
	size_t increment = hog_size;

	fseek(*fh_hog, 0, SEEK_SET);
	for(i=0;i<limit*increment;i+=increment)
	{
		fread(hogs+i, 4, increment, *fh_hog);
	}

}
