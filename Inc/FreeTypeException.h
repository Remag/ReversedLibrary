#pragma once
#include <Errors.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

class REAPI CFreeTypeException : public CException {
public:
	// Create an exception from the given code.
	explicit CFreeTypeException( int code );

	virtual CUnicodeString GetMessageText() const override;
	
private:
	// Basic error description.
	CUnicodeString errorBase;
	// Name of the FreeType module that threw an error.
	CUnicodeString moduleName;
	// Code of the error.
	int errorCode;

	void setInfoFromCode( int code );
	static CUnicodeView getModuleName( int code );
};

//////////////////////////////////////////////////////////////////////////

inline void checkFreeTypeError( int errorCode )
{
	if( errorCode != 0 ) {
		throw CFreeTypeException( errorCode );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.