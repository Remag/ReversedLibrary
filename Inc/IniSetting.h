#pragma once
#include <IniFile.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// An ini file setting wrapper that allows modifying a globally defined ini file.
// FileSource must provide a static GetIniFile() method that returns a target CIniFile.
template <class FileSource, class T>
class CIniSetting {
public:
	CIniSetting( CStringPart settingName, T defaultValue, CStringPart sectionName = CStringPart() );

	static void SaveChanges();

	T Get() const;
	void Set( T newValue );

private:
	CIniFile& fileSrc;
	T defaultValue;
	int sectionId;
	int keyId;
};

//////////////////////////////////////////////////////////////////////////

template <class FileSource, class T>
CIniSetting<FileSource, T>::CIniSetting( CStringPart settingName, T _defaultValue, CStringPart sectionName ) :
	fileSrc( FileSource::GetIniFile() ),
	defaultValue( move( _defaultValue ) )
{
	sectionId = fileSrc.GetOrCreateSectionId( sectionName );
	keyId = fileSrc.GetOrCreateKeyId( sectionId, settingName, defaultValue );
}

template <class FileSource, class T>
void CIniSetting<FileSource, T>::SaveChanges()
{
	FileSource::GetIniFile().Save();
}

template <class FileSource, class T>
T CIniSetting<FileSource, T>::Get() const
{
	return fileSrc.GetValue( sectionId, keyId, defaultValue );
}

template <class FileSource, class T>
void CIniSetting<FileSource, T>::Set( T newValue )
{
	fileSrc.SetValue( sectionId, keyId, move( newValue ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

