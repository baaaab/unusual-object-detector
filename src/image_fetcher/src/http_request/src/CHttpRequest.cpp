#include "CHttpRequest.h"

#include <algorithm>

bool CHttpRequest::_curlIsInitialised = false;
uint32_t CHttpRequest::_InstanceCount = 0;

CHttpRequest::CHttpRequest(string url) :
		_errorCode( CURLE_OK )
{
	if(_InstanceCount == 0)
	{
		initCurl();
	}

	_InstanceCount++;

	initCurl();

	_curl = curl_easy_init();

	curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(_curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:23.0) Gecko/20100101 Firefox/23.0");
	curl_easy_setopt(_curl, CURLOPT_POST, 1);
	curl_easy_setopt(_curl, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(_curl, CURLOPT_AUTOREFERER, 1);
	curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1L);
}

CHttpRequest::~CHttpRequest()
{
	curl_easy_cleanup(_curl);
	_InstanceCount--;
	if(_InstanceCount == 0)
	{
		destroyCurl();
	}
}

void CHttpRequest::initCurl()
{
	if(!_curlIsInitialised)
	{
		 curl_global_init(CURL_GLOBAL_ALL);
		 _curlIsInitialised = true;
	}
}

void CHttpRequest::destroyCurl()
{
	if(_curlIsInitialised)
	{
		curl_global_cleanup();
		_curlIsInitialised = false;
	}
}

void CHttpRequest::setRequestData(const char* data, uint32_t length)
{
	_requestData.reserve(length);
	_requestData.insert(_requestData.begin(), data, data+length);

	curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, &_requestData[0]);
	curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, _requestData.size());
}

bool CHttpRequest::performRequest()
{
  curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteFunction);
  curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &_responseData);

  return (curl_easy_perform(_curl) == 0);
}

uint32_t  CHttpRequest::getResponseSize()
{
	return _responseData.size();
}

const char* CHttpRequest::getResponse()
{
	return &_responseData[0];
}

const char* CHttpRequest::getErrorString()
{
	return curl_easy_strerror(_errorCode);
}

size_t CHttpRequest::WriteFunction(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	vector<char>* responseData = (vector<char>*)userdata;
	(*responseData).insert((*responseData).end(), ptr, ptr + size*nmemb);
	return size*nmemb;
}
