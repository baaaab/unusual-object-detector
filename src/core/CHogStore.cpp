#include "CHogStore.h"

CHogStore::CHogStore(std::string filename, uint32_t numHogs) :
	_fh( NULL )
{
	_fh = fopen(filename.c_str(), "r+b");
	if(!_fh)
	{
		printf("%s::%s Error opening hog store file: %s\n", __FILE__, __FUNCTION__, filename.c_str());
		throw 1;
	}

	reserve(numHogs);

	for (uint32_t i = 0; i < numHogs; i++)
	{
		CHog hog(_fh);
		push_back(hog);
	}
}

CHogStore::~CHogStore()
{
	fclose(_fh);
}

void CHogStore::write(uint32_t index)
{
	uint32_t fileOffset = index * CHog::GetSizeBytes();
	fseek(_fh, fileOffset, SEEK_SET);
	at(index).write(_fh);
}

void CHogStore::write(std::vector<CHog>::iterator itr)
{
	write(itr - begin());
}

void CHogStore::modelHogs(CModel* model)
{
	for(auto itr = begin(); itr != end(); ++itr)
	{
		itr->computeRCH(model);
	}
}

