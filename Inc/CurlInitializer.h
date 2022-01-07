#pragma once
#include <Redefs.h>

#ifndef RELIB_NO_INTERNET

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// LibCurl initialization mechanism.
class CCurlInitializer {
public:
	CCurlInitializer();
	~CCurlInitializer();
};

//////////////////////////////////////////////////////////////////////////

// Perform LibCurl initialization if necessary.
void REAPI InitializeCurl();

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

#endif	// RELIB_NO_INTERNET
