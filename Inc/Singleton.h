#pragma once
#include <TemplateUtils.h>
#include <Reassert.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

template <class Target>
class CSingleton {
public:
	static Target* GetInstance();

protected:
	CSingleton();
	~CSingleton();

private:
	static Target* instance;

	// Copying is prohibited.
	CSingleton( CSingleton& ) = delete;
	void operator=( CSingleton& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class Target>
Target* CSingleton<Target>::instance;

template <class Target>
CSingleton<Target>::CSingleton()
{
	staticAssert( ( Types::IsDerivedFrom<Target, CSingleton<Target>>::Result ) );
	assert( instance == 0 );
	instance = static_cast<Target*>( this );
}

template <class Target>
CSingleton<Target>::~CSingleton()
{
	assert( instance == this );
	instance = nullptr;
}

template <class Target>
Target* CSingleton<Target>::GetInstance()
{
	assert( instance != nullptr );
	return instance;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

