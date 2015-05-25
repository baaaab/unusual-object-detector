#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#include <settings.h>
#include "../utils/CSettingsRegistry.h"
#include "CHog.h"
#include "CModel.h"
#include "CUnusualObjectDetector.h"

namespace
{
bool shutdownRequested = false;
}

void sigHandler(int signo);

void printUsage(const char* program)
{
	printf("Usage: %s project_file [first_run_args]\n", program);
	printf("\t First run args:\n");
	printf("\t\t -n number_of_comparison_images [> 100]\n");
	printf("\t\t -i image_folder\n");
	printf("\t\t [-t num_threads = 1]\n");
}

int main(int argc, char* argv[])
{
	bool configurationIsValid = false;

	if (argc == 1)
	{
		printUsage(argv[0]);
		return 1;
	}

	if (argc == 2)
	{
		//only 1 arg - must be a valid project XML file
		CSettingsRegistry registry(argv[1]);
		try
		{
			registry.getUInt32("core", "programCounter");
			registry.getUInt32("core", "imageCount");
			registry.getString("core", "imageDir");
			registry.getString("core", "numThreads");
		}
		catch (...)
		{
			printf("Error: invalid project settings file (%s)\n", argv[1]);
			printUsage(argv[0]);
			return 1;
		}
		configurationIsValid = true;
	}
	else
	{
		//new project setup

		//check if this is a valid configuration - it will be overwritten if it is
		CSettingsRegistry registry(argv[1]);
		configurationIsValid = true;
		try
		{
			registry.getUInt32("core", "programCounter");
		}
		catch (...)
		{
			configurationIsValid = false;
		}

		if (configurationIsValid)
		{
			printf("Too many arguments or project file already exists!\n");
			printUsage(argv[0]);
			return 1;
		}

		std::string imageUrl;
		uint32_t imageCount = 0;
		std::string imageDir;
		uint32_t numThreads = 1;

		int c;
		while ((c = getopt(argc-1, argv+1, "n:i:t:")) != -1)
		{
			switch (c)
			{
			case 'n':
				imageCount = strtoul(optarg, NULL, 10);
				break;
			case 'i':
				imageDir = optarg;
				break;
			case 't':
				numThreads = strtoul(optarg, NULL, 10);
				break;
			case '?':
				if (optopt == 'n' || optopt == 'i' || optopt == 't')
				{
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				}
				else if (isprint(optopt))
				{
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				}
				else
				{
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
				}
				return 1;
			default:
				abort();
			}
		}

		struct stat pathInfo;
		if (stat(imageDir.c_str(), &pathInfo) == -1)
		{
			printf("Error accessing image directory: %s\n", imageDir.c_str());
			perror(__FILE__);
			printUsage(argv[0]);
			return 1;
		}
		else
		{
			if (!S_ISDIR(pathInfo.st_mode))
			{
				printf("Error: Not a directory: %s\n", imageDir.c_str());
				printUsage(argv[0]);
				return 1;
			}
			if (!pathInfo.st_mode & S_IWUSR)
			{
				printf("Error: Cannot write to directory: %s\n", imageDir.c_str());
				printUsage(argv[0]);
				return 1;
			}
		}

		registry.setUInt32("core", "programCounter", 0);
		registry.setUInt32("core", "imageCount", imageCount);
		registry.setString("core", "imageDir", imageDir);
		registry.setUInt32("core", "numThreads", numThreads);

		//create dirs/files
		std::string modelDir = imageDir.append("/model");

		mkdir(modelDir.c_str(), 0775);

		std::string hogStoreFilename = modelDir + std::string("/hogs.dat");
		FILE* fh = fopen(hogStoreFilename.c_str(), "w");
		CHog emptyHog(HOG_CELL_SIZE, HOG_NUM_CELLS, NULL);
		for (uint32_t i = 0; i < imageCount; i++)
		{
			emptyHog.write(fh);
			printf("Initialising hog %u\n", i);
		}

		fclose(fh);

		CModel defaultModel(HOG_NUM_CELLS, &registry);
		defaultModel.saveToRegistry();

		std::string scoresDistributionFilename = modelDir + std::string("/scores.dat");
		fh = fopen(scoresDistributionFilename.c_str(), "w");
		float f = 0;
		for (uint32_t i = 0; i < imageCount; i++)
		{
			fwrite(&f, 1, sizeof(float), fh);
		}
		fclose(fh);

		configurationIsValid = true;
	}

	if(!configurationIsValid)
	{
		printf("Configuration is invalid!\n");
		return 1;
	}

	signal(SIGINT, sigHandler);

	CUnusualObjectDetector* uod = new CUnusualObjectDetector(argv[1]);

	while (!shutdownRequested)
	{
		usleep(250000);
	}

	delete uod;

	return 0;
}

void sigHandler(int signo)
{
	switch (signo)
	{
	case SIGINT:
		printf("SHUTDOWN REQUESTED\n");
		shutdownRequested = true;
		break;
	}
}
