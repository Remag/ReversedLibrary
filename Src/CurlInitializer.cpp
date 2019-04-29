#include <CurlInitializer.h>
#include <LibCurl\curl.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CCurlInitializer::CCurlInitializer()
{
	curl_global_init( CURL_GLOBAL_ALL );
}

CCurlInitializer::~CCurlInitializer()
{
	curl_global_cleanup();
}

//////////////////////////////////////////////////////////////////////////

void InitializeCurl()
{
	static CCurlInitializer initClass;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
