#pragma once
// Functions and classes that work with exceptions and other errors.

#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <BaseString.h>
#include <StrConversions.h>
#include <Remath.h>
#include <ExplicitCopy.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

void REAPI GenerateLastErrorException( DWORD errorCode );
void REAPI ThrowMemoryException();
void REAPI StopExecutionIfNecessary();

inline void checkLastError( bool condition )
{
	if( !condition ) {
		DWORD errorCode = GetLastError();
		if( errorCode == ERROR_NOT_ENOUGH_MEMORY ) {
			ThrowMemoryException();
		} else {
			GenerateLastErrorException( errorCode );
		}
	}
}

template <typename Dest, typename Src>
inline Dest check_cast( Src* src )
{
	Dest result = dynamic_cast<Dest>( src );
	assert( result != nullptr );
	return result;
}

template <class Dest, class Src>
inline Dest check_cast( CSharedPtr<Src>& src )
{
	Dest result = dynamic_cast<Dest>( src.Ptr() );
	assert( result != nullptr );
	return result;
}

template <class Dest, class Src>
inline Dest check_cast( const CSharedPtr<Src>& src )
{
	Dest result = dynamic_cast<Dest>( src.Ptr() );
	assert( result != nullptr );
	return result;
}

template <typename Dest, typename Src>
inline Dest check_cast( Src* src, const CError& err )
{
	Dest result = dynamic_cast<Dest>( src );
	check( result != nullptr, err );
	return result;
}

template <class Dest, class Src>
inline Dest check_cast( CSharedPtr<Src>& src, const CError& err )
{
	Dest result = dynamic_cast<Dest>( src.Ptr() );
	check( result != nullptr, err );
	return result;
}

template <class Dest, class Src, class Counter>
inline Dest check_cast( const CSharedPtr<Src>& src, const CError& err )
{
	Dest result = dynamic_cast<Dest>( src.Ptr() );
	check( result != nullptr, err );
	return result;
}

//////////////////////////////////////////////////////////////////////////

// Base class for all exceptions.
class REAPI CException {
public:
	CException();
	virtual ~CException();

	virtual CString GetMessageText() const = 0;
};

//////////////////////////////////////////////////////////////////////////

// Exception that is thrown when an assertion fails. Contains information about the position of the failed assertion in the code.
class REAPI CInternalException : public CException {
public:
	explicit CInternalException( CString errorText );

	// CException.
	virtual CString GetMessageText() const override final;

private:
	// Description of the internal error.
	CString errorText;
};

//////////////////////////////////////////////////////////////////////////

// Memory shortage exception.
class REAPI CMemoryException : public CException {
public:
	CMemoryException();

	// CException.
	virtual CString GetMessageText() const override final;

	// Message that is shown when the exception is thrown.
	static const CStringView NotEnoughMemoryMessage;
};

void REAPI checkMemoryError( bool condition );

//////////////////////////////////////////////////////////////////////////

// Class representing an error message.
class REAPI CError {
public:
	explicit CError( CStringView name ) : messageStr( name ) {}

	CStringView GetMessageText() const 
		{ return messageStr; }

private:
	CStringView messageStr;

	// Copying is prohibited.
	CError( CError& ) = delete;
	void operator=( CError& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// Exception thrown by the check function. A simple wrapper around CError.
class REAPI CCheckException : public CException {
public:
	template <class... Params>
	CCheckException( const CError& err, Params&&... params );

	// CException.
	virtual CString GetMessageText() const; 

	const CError& Error() const
		{ return err; }
	CStringView FirstParameter() const
		{ return params[0]; }
	CStringView SecondParameter() const
		{ return params[1]; }
	CStringView ThirdParameter() const
		{ return params[2]; }

	void SetFirstParam( CStringPart newValue )
		{ params[0] = newValue; }
	void SetSecondParam( CStringPart newValue )
		{ params[1] = newValue; }
	void SetThirdParam( CStringPart newValue )
		{ params[2] = newValue; }

private:
	// An error object. It is assumed that all errors are static.
	const CError& err;
	// Optional error parameters.
	CString params[3];

	static void initParam( CString* params );
	template <class FirstParam, class...RestParams>
	static void initParam( CString* params, FirstParam&& first, RestParams&&... rest );
};

template <class... Params>
CCheckException::CCheckException( const CError& _err, Params&&... _params ) :
	err( _err )
{
	static_assert( sizeof...( _params ) <= 3, "Too many parameters for a check." );
	initParam( params, forward<Params>( _params )... );
}

inline void CCheckException::initParam( CString* )
{
}

template <class FirstParam, class...RestParams>
void CCheckException::initParam( CString* params, FirstParam&& first, RestParams&&... rest )
{
	*params = Str( forward<FirstParam>( first ) );
	initParam( params + 1, forward<RestParams>( rest )... );
}

template <class... Params>
void GenerateCheck( const CError& err, Params&&... params )
{
	StopExecutionIfNecessary();
	throw CCheckException( err, forward<Params>( params )... );
}

template <class... Params>
inline void check( bool condition, const CError& err, Params&&... params )
{
	if( !condition ) {
		GenerateCheck( err, forward<Params>( params )... );
	}
}

//////////////////////////////////////////////////////////////////////////

// A wrapper around GetLastError function.
// This exception is generated by checkLastError.
class REAPI CLastErrorException : public CException {
public:
	explicit CLastErrorException( DWORD errorCode );
	
	// CException.
	virtual CString GetMessageText() const;

	DWORD ErrorCode() const
		{ return errorCode;	}
	static CString GetErrorText( DWORD errorCode );

private:
	const DWORD errorCode;
};

//////////////////////////////////////////////////////////////////////////

// Exception thrown when working with files fails.
class REAPI CFileException : public CException {
public:
	enum TFileExceptionType {
		FET_None,	// default value
		FET_General,	// general error.
		FET_FileNotFound,	// unable to find a file with the given name.
		FET_InvalidFile,	// file is invalid.
		FET_FileTooBig,	// file size is too large.
		FET_BadPath,	// the given path is invalid.
		FET_AlreadyExists,	// an object with the given name already exists.
		FET_AccessDenied,	// access to the file is denied.
		FET_SharingViolation,	// used sharing mode is denied for the given file.
		FET_DiskFull,	// not enough free space.
		FET_HardwareError,	// hardware IO exception occurred.
		FET_EarlyEnd	// unexpected end of file.
	};

	CFileException( DWORD errorCode, CStringPart fileName );
	CFileException( TFileExceptionType type, CStringPart fileName );
	CFileException( const CFileException& other ) : errorCode( other.errorCode ), type( other.type ), fileName( copy( other.fileName ) ) {}

	// CException.
	virtual CString GetMessageText() const override;

	DWORD ErrorCode() const
		{ return errorCode; }
	TFileExceptionType Type() const
		{ return type; }
	CStringView FileName() const
		{ return fileName; }

	static TFileExceptionType GetErrorType( DWORD errorCode );
	static CString GetErrorText( TFileExceptionType type, CStringView name, int code );

private:
	const DWORD errorCode;
	const TFileExceptionType type;
	const CString fileName;
};

void ThrowFileException( DWORD lastErrorCode, CStringPart fileName );
void ThrowFileException( CFileException::TFileExceptionType type, CStringPart fileName );

//////////////////////////////////////////////////////////////////////////

// Generic exception thrown by a file wrapper class.
class REAPI CFileWrapperException : public CException {
public:
	CFileWrapperException( CStringPart _fileName, CStringPart _additionalInfo ) :
		fileName( _fileName ), additionalInfo( _additionalInfo ) {}
	CFileWrapperException( const CFileWrapperException& other ) : fileName( copy( other.fileName ) ), additionalInfo( copy( other.additionalInfo ) ) {}

	virtual CString GetMessageText() const final override
		{ return GetMessageTemplate().SubstParam( fileName, additionalInfo ); }
	
	// Get the template for exception's message text.
	// The template must have two parameters: first one is the name of the file and second one is additional information.
	virtual CString GetMessageTemplate() const = 0;

private:
	CString fileName;
	CString additionalInfo;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.