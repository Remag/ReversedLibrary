// Based heavily on GifDecoder - a public domain gif file reader.

#pragma once
#include <Redefs.h>
#include <Errors.h>
#include <PtrOwner.h>
#include <DynamicBitset.h>


namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

class CGifInternalException : public CException {
public:
	explicit CGifInternalException( CUnicodeView _errorStr ) : errorStr( _errorStr ) {}

	virtual CUnicodeString GetMessageText() const override final
	{ return copy( errorStr ); }

private:
	CUnicodeString errorStr;
};

struct CGifPalette {
	int size;
	uint8_t colors[0x100 * 3];
};

struct CGiffGce {
	// Frame duration in hundredths of a second.
	uint16_t delay;
	uint8_t tindex;
	uint8_t disposal;
	int input;
	int transparency;
};

struct CGiffBuffer {
	CArrayView<BYTE> GifData;
	int Pos;
};

struct CGiffDecodeData {
	CGiffBuffer fd;
	int anim_start;
	uint16_t width, height;
	uint16_t depth;
	uint16_t loop_count;
	CGiffGce gce;
	CGifPalette* palette;
	CPtrOwner<CGifPalette> lct, gct;
	void ( *plain_text )( CGiffDecodeData* gif, uint16_t tx, uint16_t ty, uint16_t tw, uint16_t th, uint8_t cw, uint8_t ch, uint8_t fg, uint8_t bg );
	void ( *comment )( CGiffDecodeData* gif );
	void ( *application )( CGiffDecodeData *gif, char id[8], char auth[3] );
	uint16_t fx, fy, fw, fh;
	uint8_t bgindex;
	uint8_t *canvas, *frame;
	CArray<uint8_t> ColorData;
	CDynamicBitSet<> TransparencyMask;
};

//////////////////////////////////////////////////////////////////////////

CGiffDecodeData gd_open_gif( CGiffBuffer buffer );
int gd_get_frame( CGiffDecodeData* gif );
void gd_render_frame( CGiffDecodeData* gif, uint8_t* buffer, CDynamicBitSet<>& transparencyMask );
void gd_rewind( CGiffDecodeData* gif );

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.