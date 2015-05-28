#ifndef SRC_CORE_CHOGSTORE_H_
#define SRC_CORE_CHOGSTORE_H_

#include <cstdint>
#include <cstdio>
#include <vector>

#include "CHog.h"

class CHogStore : public std::vector<CHog>
{
public:
	CHogStore(std::string, uint32_t numHogs);
	virtual ~CHogStore();

	void write(uint32_t index);
	void write(std::vector<CHog>::iterator itr);

private:
	FILE* _fh;

};

#endif /* SRC_CORE_CHOGSTORE_H_ */
