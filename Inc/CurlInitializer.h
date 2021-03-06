#pragma once
#include <Redefs.h>

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

