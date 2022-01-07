#include <GifFile.h>
#include <DynamicBitset.h>
#include <FileOwners.h>
#include <StaticArray.h>

#include <gifdec.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CGifFile::CGifFile( CUnicodePart _fileName ) :
	fileName( _fileName )
{
}

void CGifFile::Read( CAnimatedImageData& result ) const
{
	assert( result.Frames.IsEmpty() );

	CFileReader file( fileName, FCM_OpenExisting );
	CArray<BYTE> fileData;
	const int fileLength = file.GetLength32();
	fileData.IncreaseSize( fileLength );
	file.Read( fileData.Ptr(), fileLength );

	doReadRawData( fileName, fileData, result );
}

void CGifFile::ReadRawData( CArrayView<BYTE> gifData, CAnimatedImageData& result )
{
	doReadRawData( CUnicodePart(), gifData, result ); 
}

void CGifFile::doReadRawData( CUnicodePart fileName, CArrayView<BYTE> gifData, CAnimatedImageData& result )
{
	try {
		const RelibInternal::CGiffBuffer buffer{ gifData, 0 };
		auto decodeData = RelibInternal::gd_open_gif( buffer );
		readGifFrames( decodeData, result.Frames );
		result.ImageSize.X() = decodeData.width;
		result.ImageSize.Y() = decodeData.height;
	} catch( RelibInternal::CGifInternalException& e ) {
		throw CGifException( fileName, e.GetMessageText() );
	}
}

void CGifFile::readGifFrames( RelibInternal::CGiffDecodeData& decodeData, CArray<CImageFrameData>& result )
{
	const auto width = decodeData.width;
	const auto height = decodeData.height;
	const auto frameArea = width * height;
	CStaticArray<BYTE> frameColorData;
	CDynamicBitSet<> frameTransparencyMask;
	frameColorData.ResetSize( frameArea * 3 );
	frameTransparencyMask.ReserveBuffer( frameArea );
	int currentEndTime = 0;
	while( gd_get_frame( &decodeData ) == 1 ) {
		auto& newFrameData = result.Add();
		gd_render_frame( &decodeData, frameColorData.Ptr(), frameTransparencyMask );
		currentEndTime += getFrameDelay( decodeData );
		newFrameData.Colors.IncreaseSize( frameArea );
		newFrameData.FrameEndTimeMs = currentEndTime;
		copyColorData( frameColorData, width, height, frameTransparencyMask, newFrameData.Colors );
	}
}

void CGifFile::copyColorData( CArrayView<BYTE> frameData, int width, int height, const CDynamicBitSet<>& transparencyMask, CArray<CColor>& result )
{
	for( int y = 0; y < height; y++ ) {
		for( int x = 0; x < width; x++ ) {
			const auto srcPos = y * width + x;
			const auto srcBytePos = 3 * srcPos;
			const auto destPos = ( height - 1 - y ) * width + x;
			const auto srcColor = transparencyMask.Has( srcPos ) ? CColor( 0, 0, 0, 0 ) : createColor( frameData, srcBytePos );
			result[destPos] = srcColor;
		}
	}
}

CColor CGifFile::createColor( CArrayView<BYTE> frameColors, int framePos )
{
	// Fill the color structure as if it was stored in an RBG format.
	// This matches the output of the png file wrapper.
	return CColor( frameColors[framePos + 2], frameColors[framePos + 1], frameColors[framePos] );
}

int CGifFile::getFrameDelay( RelibInternal::CGiffDecodeData& decodeData )
{
	const auto decodeDelay = decodeData.gce.delay;
	// Delays of 0 and 1 are commonly interpreted as a delay of 10.
	return decodeDelay <= 1 ? 100 : decodeDelay * 10;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

