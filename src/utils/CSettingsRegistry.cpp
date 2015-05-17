#include "CSettingsRegistry.h"

#include <stdlib.h>

CSettingsRegistry::CSettingsRegistry(const char* xmlFile) :
		_doc( NULL),
		_xmlFileName(xmlFile)
{
	_doc = new tinyxml2::XMLDocument();

	_doc->LoadFile(_xmlFileName.c_str());
}

CSettingsRegistry::~CSettingsRegistry()
{
	delete _doc;
}

void CSettingsRegistry::setUInt32(const char* group, const char* name, uint32_t value)
{
	tinyxml2::XMLElement* groupNode = _doc->FirstChildElement(group);
	if (!groupNode)
	{
		groupNode = _doc->NewElement(group);
		_doc->LinkEndChild(groupNode);
	}

	tinyxml2::XMLElement* textNode = groupNode->FirstChildElement(name);
	if (!textNode)
	{
		//create new
		textNode = _doc->NewElement(name);
		groupNode->LinkEndChild(textNode);
	}
	textNode->SetText(value);

	_doc->SaveFile(_xmlFileName.c_str(), false);
}

uint32_t CSettingsRegistry::getUInt32(const char* group, const char* name)
{
	tinyxml2::XMLElement* groupNode = _doc->FirstChildElement(group);
	if (!groupNode)
	{
		throw 1;
	}
	tinyxml2::XMLElement* textNode = groupNode->FirstChildElement(name);
	if (!textNode)
	{
		throw 1;
	}
	return strtoul(textNode->GetText(), NULL, 10);
}

void CSettingsRegistry::setInt32(const char* group, const char* name, int32_t value)
{
	tinyxml2::XMLElement* groupNode = _doc->FirstChildElement(group);
	if (!groupNode)
	{
		groupNode = _doc->NewElement(group);
		_doc->LinkEndChild(groupNode);
	}

	tinyxml2::XMLElement* textNode = groupNode->FirstChildElement(name);
	if (!textNode)
	{
		//create new
		textNode = _doc->NewElement(name);
		groupNode->LinkEndChild(textNode);
	}
	textNode->SetText(value);

	_doc->SaveFile(_xmlFileName.c_str(), false);
}

int32_t CSettingsRegistry::getInt32(const char* group, const char* name)
{
	tinyxml2::XMLElement* groupNode = _doc->FirstChildElement(group);
	if (!groupNode)
	{
		throw 1;
	}
	tinyxml2::XMLElement* textNode = groupNode->FirstChildElement(name);
	if (!textNode)
	{
		throw 1;
	}
	return strtol(textNode->GetText(), NULL, 10);
}

void CSettingsRegistry::setFloat(const char* group, const char* name, float value)
{
	tinyxml2::XMLElement* groupNode = _doc->FirstChildElement(group);
	if (!groupNode)
	{
		groupNode = _doc->NewElement(group);
		_doc->LinkEndChild(groupNode);
	}

	tinyxml2::XMLElement* textNode = groupNode->FirstChildElement(name);
	if (!textNode)
	{
		//create new
		textNode = _doc->NewElement(name);
		groupNode->LinkEndChild(textNode);
	}
	textNode->SetText(value);

	_doc->SaveFile(_xmlFileName.c_str(), false);
}

float CSettingsRegistry::getFloat(const char* group, const char* name)
{
	tinyxml2::XMLElement* groupNode = _doc->FirstChildElement(group);
	if (!groupNode)
	{
		throw 1;
	}
	tinyxml2::XMLElement* textNode = groupNode->FirstChildElement(name);
	if (!textNode)
	{
		throw 1;
	}
	return strtof(textNode->GetText(), NULL);
}

void CSettingsRegistry::setString(const char* group, const char* name, std::string value)
{
	tinyxml2::XMLElement* groupNode = _doc->FirstChildElement(group);
	if (!groupNode)
	{
		groupNode = _doc->NewElement(group);
		_doc->LinkEndChild(groupNode);
	}

	tinyxml2::XMLElement* textNode = groupNode->FirstChildElement(name);
	if (!textNode)
	{
		//create new
		textNode = _doc->NewElement(name);
		groupNode->LinkEndChild(textNode);
	}
	textNode->SetText(value.c_str());

	_doc->SaveFile(_xmlFileName.c_str(), false);
}

std::string CSettingsRegistry::getString(const char* group, const char* name)
{
	tinyxml2::XMLElement* groupNode = _doc->FirstChildElement(group);
	if (!groupNode)
	{
		throw 1;
	}
	tinyxml2::XMLElement* textNode = groupNode->FirstChildElement(name);
	if (!textNode)
	{
		throw 1;
	}
	return std::string(textNode->GetText());
}

