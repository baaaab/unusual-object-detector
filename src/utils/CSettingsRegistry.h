#ifndef CSETTINGSREGISTRY_H_
#define CSETTINGSREGISTRY_H_

#include "tinyxml2.h"

#include <cstdint>
#include <string>

class CSettingsRegistry
{
public:
	CSettingsRegistry(const char* xmlFile);
	virtual ~CSettingsRegistry();

	void setUInt32(const char* group, const char* name, uint32_t value);
	uint32_t getUInt32(const char* group, const char* name);

	void setInt32(const char* group, const char* name, int32_t value);
	int32_t getInt32(const char* group, const char* name);

	void setFloat(const char* group, const char* name, float value);
	float getFloat(const char* group, const char* name);

	void setString(const char* group, const char* name, std::string value);
	std::string getString(const char* group, const char* name);

private:

	tinyxml2::XMLDocument* _doc;
	std::string _xmlFileName;
};

#endif /* CSETTINGSMANAGER_H_ */
