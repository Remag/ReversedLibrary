#include <Errors.h>
#include <Reutils.h>
#include <StrConversions.h>

namespace Relib {

// Creates a break point depending on the program running mode.
// This function is defined as global but not exported.
void StopExecutionIfNecessary()
{
	TLibraryDebugMode mode = GetLibraryDebugMode();
	if( mode == LDM_DebugStop ) {
		ProgramDebuggerBreak();
	} else if( mode == LDM_AlwaysStop ) {
		ProgramBreakPoint();
	}
}

void GenerateLastErrorException( DWORD errorCode )
{
	StopExecutionIfNecessary();
	throw CLastErrorException( errorCode );
}

//////////////////////////////////////////////////////////////////////////

CInternalException::CInternalException( CString _errorText )
	: errorText( move( _errorText ) )
{
}

CInternalException::CInternalException( const CInternalException& other )
	: errorText( copy( other.errorText ) )
{
}

CString CInternalException::GetMessageText() const
{
	return copy( errorText );
}

//////////////////////////////////////////////////////////////////////////

CMemoryException::CMemoryException()
{
}

CString CMemoryException::GetMessageText() const
{
	return Str( NotEnoughMemoryMessage );
}

void ThrowMemoryException()
{
	StopExecutionIfNecessary();
	throw CMemoryException();
}

void checkMemoryError( bool condition )
{
	if( !condition ) {
		ThrowMemoryException();
	}
}

//////////////////////////////////////////////////////////////////////////

CCheckException::CCheckException( const CCheckException& other )
	: err( other.err ),
	  firstParam( copy( other.firstParam ) ),
	  secondParam( copy( other.secondParam ) ),
	  thirdParam( copy( other.thirdParam ) )
{
}

CString CCheckException::GetMessageText() const
{
	return err.GetMessageText().SubstParam( firstParam, secondParam, thirdParam );
}

//////////////////////////////////////////////////////////////////////////

CLastErrorException::CLastErrorException( DWORD _errorCode )
	: errorCode( _errorCode )
{
}

CString CLastErrorException::GetMessageText() const
{
	return GetErrorText( errorCode );
}

extern const CStringView UnknownLastError;
CString CLastErrorException::GetErrorText( DWORD errorCode )
{
	if( errorCode == 0 ) {
		return UnknownLastError.SubstParam( L"0" );
	}

	wchar_t* text = nullptr;
	const DWORD formatResult = ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
		0, errorCode, LANG_NEUTRAL, reinterpret_cast<wchar_t*>( &text ), 0, 0 );

	auto result = ( text == nullptr || formatResult == 0 ) ? UnknownLastError.SubstParam( Str( numeric_cast<int>( errorCode ), 16 ) ) : Str( text );

	if( text != nullptr ) {
		::LocalFree( static_cast<HLOCAL>( text ) );
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////

CFileException::CFileException( DWORD _errorCode, CStringPart _fileName )
	: errorCode( _errorCode ),
	  fileName( _fileName ),
	  type( GetErrorType( _errorCode ) )
{
}

CFileException::CFileException( TFileExceptionType _type, CStringPart _fileName )
	: errorCode( 0 ),
	  fileName( _fileName ),
	  type( _type )
{
}

CString CFileException::GetMessageText() const
{
	return GetErrorText( Type(), FileName(), errorCode );
}

CFileException::TFileExceptionType CFileException::GetErrorType( DWORD errorCode )
{
	switch( errorCode ) {
		case ERROR_FILE_NOT_FOUND:
		case ERROR_INVALID_HANDLE:
		case ERROR_NO_MORE_FILES:
		case ERROR_DISK_CHANGE:
			return FET_FileNotFound;
		case ERROR_BAD_FORMAT:
		case ERROR_NOT_DOS_DISK:
		case ERROR_BAD_REM_ADAP:
		case ERROR_BAD_DEV_TYPE:
		case ERROR_INVALID_TARGET_HANDLE:
		case ERROR_INVALID_ORDINAL:
		case ERROR_INVALID_EXE_SIGNATURE:
		case ERROR_BAD_EXE_FORMAT:
			return FET_InvalidFile;
		case ERROR_PATH_NOT_FOUND:
		case ERROR_INVALID_NAME:
		case ERROR_INVALID_LEVEL:
		case ERROR_NO_VOLUME_LABEL:
		case ERROR_INVALID_DRIVE:
		case ERROR_WRONG_DISK:
		case ERROR_NOT_SAME_DEVICE:
		case ERROR_DUP_NAME:
		case ERROR_DEV_NOT_EXIST:
		case ERROR_BAD_NETPATH:
		case ERROR_BAD_NET_NAME:
		case ERROR_SHARING_PAUSED:
		case ERROR_ALREADY_ASSIGNED:
		case ERROR_BUFFER_OVERFLOW:
		case ERROR_DIR_NOT_ROOT:
		case ERROR_LABEL_TOO_LONG:
		case ERROR_BAD_PATHNAME:
		case ERROR_FILENAME_EXCED_RANGE:
		case ERROR_META_EXPANSION_TOO_LONG:
		case ERROR_DIRECTORY:
			return FET_BadPath;
		case ERROR_ALREADY_EXISTS:
		case ERROR_FILE_EXISTS:
			return FET_AlreadyExists;
		case ERROR_ACCESS_DENIED:
		case ERROR_BUSY:
		case ERROR_CANNOT_MAKE:
		case ERROR_INVALID_ACCESS:
		case ERROR_INVALID_PASSWORD:
		case ERROR_NETWORK_ACCESS_DENIED:
		case ERROR_NETWORK_BUSY:
		case ERROR_NETNAME_DELETED:
		case ERROR_BAD_NET_RESP:
		case ERROR_REQ_NOT_ACCEP:
		case ERROR_SWAPERROR:
		case ERROR_WRITE_PROTECT:
			return FET_AccessDenied;
		case ERROR_SHARING_VIOLATION:
			return FET_SharingViolation;
		case ERROR_ADAP_HDW_ERR:
		case ERROR_BAD_UNIT:
		case ERROR_BAD_COMMAND:
		case ERROR_CRC:
		case ERROR_INVALID_CATEGORY:
		case ERROR_IO_INCOMPLETE:
		case ERROR_IO_PENDING:
		case ERROR_NET_WRITE_FAULT:
		case ERROR_NOT_READY:
		case ERROR_OPERATION_ABORTED:
		case ERROR_UNEXP_NET_ERR:
			return FET_HardwareError;
		case NO_ERROR:
			assert( false );
			return FET_None;
		default:
			return FET_General;
	}
}

extern const CStringView GeneralFileError;
extern const CStringView FileNotFoundError;
extern const CStringView InvalidFileError;
extern const CStringView FileTooBigError;
extern const CStringView BadPathFileError;
extern const CStringView ObjectAlreadyExistsError;
extern const CStringView AccessDeniedError;
extern const CStringView SharingViolationFileError;
extern const CStringView DiskFullError;
extern const CStringView EarlyEndFileError;
extern const CStringView HardwareFileError;
CString CFileException::GetErrorText( CFileException::TFileExceptionType type, CStringView name, int code )
{
	switch( type ) {
		case FET_FileNotFound:
			return FileNotFoundError.SubstParam( name );
		case FET_InvalidFile:
			return InvalidFileError.SubstParam( name );
		case FET_FileTooBig:
			return FileTooBigError.SubstParam( name );
		case FET_BadPath:
			return BadPathFileError.SubstParam( name );
		case FET_AlreadyExists:
			return ObjectAlreadyExistsError.SubstParam( name );
		case FET_AccessDenied:
			return AccessDeniedError.SubstParam( name );
		case FET_SharingViolation:
			return SharingViolationFileError.SubstParam( name );
		case FET_DiskFull:
			return DiskFullError.SubstParam( name );
		case FET_EarlyEnd:
			return EarlyEndFileError.SubstParam( name );
		case FET_HardwareError:
			return HardwareFileError.SubstParam( name );
		case FET_None:
			assert( false );
		default:
			return GeneralFileError.SubstParam( name, code );
	}
}

void ThrowFileException( DWORD lastErrorCode, CStringPart fileName )
{
	throw CFileException( lastErrorCode, fileName );
}

void ThrowFileException( CFileException::TFileExceptionType type, CStringPart fileName )
{
	throw CFileException( type, fileName );
}

//////////////////////////////////////////////////////////////////////////

}	 // namespace Relib.
