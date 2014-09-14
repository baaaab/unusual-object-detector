#ifndef CHTTPREQUEST_H_
#define CHTTPREQUEST_H_

#include <curl/curl.h>
#include <vector>
#include <inttypes.h>

#include <string>

using std::string;
using std::vector;

class CHttpRequest
{
	public:
		                 CHttpRequest         (string url);
		virtual          ~CHttpRequest        ();

		static void      initCurl             ();
		static void      destroyCurl          ();

		void             setRequestData       (const char* data, uint32_t length);

		bool             performRequest       ();

		uint32_t         getResponseSize      ();
		const char*      getResponse          ();

		const char*      getErrorString       ();

	private:

		static size_t    WriteFunction        (char *ptr, size_t size, size_t nmemb, void *userdata);

		CURL*            _curl;
		vector<char>     _responseData;
		vector<char>     _requestData;
		CURLcode         _errorCode;

		static bool      _curlIsInitialised;
		static uint32_t  _InstanceCount;



};

#endif /* CHTTPREQUEST_H_ */
