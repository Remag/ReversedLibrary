#pragma once
#include <Singleton.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Mechanism for initializing and cleaning up Reversed Library data.
class REAPI CRelibInitializer : public CSingleton<CRelibInitializer> {
public:
	CRelibInitializer();
	~CRelibInitializer();

private:
	void initFreeType();
	void initAppDataPath();
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

