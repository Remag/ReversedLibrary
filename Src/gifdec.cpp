// Based heavily on GifDecoder - a public domain gif file reader.
#include <Redefs.h>

#ifndef RELIB_NO_IMAGELIB

#include <gifdec.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

struct CGifEntry {
	uint16_t length;
	uint16_t prefix;
	uint8_t suffix;
};

struct CGifTable {
	int bulk;
	int nentries;
	CArray<CGifEntry> entries;
};

//////////////////////////////////////////////////////////////////////////

static void throwGifError( CStringPart errorStr )
{
	throw CGifInternalException( errorStr );
}

static void read( CGiffBuffer& buffer, void* dest, int byteCount )
{
	::memcpy( dest, buffer.GifData.Ptr() + buffer.Pos, byteCount );
	buffer.Pos += byteCount;
}

static uint16_t read_num( CGiffBuffer& buffer )
{
	const auto pos = buffer.Pos;
	uint8_t bytes[2]{ buffer.GifData[pos], buffer.GifData[pos + 1] };
	buffer.Pos += 2;
	return bytes[0] + ( ( (uint16_t) bytes[1] ) << 8 );
}

CGiffDecodeData gd_open_gif( CGiffBuffer fd )
{
	uint8_t sigver[3];
	uint16_t width, height, depth;
	uint8_t fdsz, bgidx, aspect;
	int gct_sz;
	CGiffDecodeData gif{};

	/* Header */
	read( fd, sigver, 3 );
	if( ::memcmp( sigver, "GIF", 3 ) != 0 ) {
		throwGifError( "invalid signature" );
	}
	/* Version */
	read( fd, sigver, 3 );
	if( ::memcmp( sigver, "89a", 3 ) != 0 ) {
		throwGifError( "invalid version" );
	}
	/* Width x Height */
	width = read_num( fd );
	height = read_num( fd );
	/* FDSZ */
	read( fd, &fdsz, 1 );
	/* Presence of GCT */
	if( !( fdsz & 0x80 ) ) {
		throwGifError( "no global color table\n" );
	}
	/* Color Space's Depth */
	depth = ( ( fdsz >> 4 ) & 7 ) + 1;
	/* Ignore Sort Flag. */
	/* GCT Size */
	gct_sz = 1 << ( ( fdsz & 0x07 ) + 1 );
	/* Background Color Index */
	read( fd, &bgidx, 1 );
	/* Aspect Ratio */
	read( fd, &aspect, 1 );
	/* Create gd_GIF Structure. */
	gif.ColorData.IncreaseSize( 4 * width * height );
	gif.width = width;
	gif.height = height;
	gif.depth = depth;

	gif.lct = CreateOwner<CGifPalette>();
	/* Read GCT */
	gif.gct = CreateOwner<CGifPalette>();
	gif.gct->size = gct_sz;
	read( fd, gif.gct->colors, 3 * gif.gct->size );
	gif.palette = gif.gct;
	gif.bgindex = bgidx;
	gif.canvas = (uint8_t*) gif.ColorData.Ptr();
	gif.frame = &gif.canvas[3 * width * height];
	if( gif.bgindex ) {
		::memset( gif.frame, gif.bgindex, gif.width * gif.height);
	}
	gif.anim_start = fd.Pos;
	gif.fd = fd;

	gif.TransparencyMask.ReserveBuffer( width * height );
	gif.TransparencyMask.FillWithOnes();

	return gif;
}

static void discard_sub_blocks( CGiffDecodeData* gif )
{
	uint8_t size;

	do {
		read( gif->fd, &size, 1) ;
		gif->fd.Pos += size;
	} while( size );
}

static void read_plain_text_ext( CGiffDecodeData* gif )
{
	if( gif->plain_text ) {
		uint16_t tx, ty, tw, th;
		uint8_t cw, ch, fg, bg;
		int sub_block;
		gif->fd.Pos += 1;  /* block size = 12 */
		tx = read_num( gif->fd );
		ty = read_num( gif->fd );
		tw = read_num( gif->fd );
		th = read_num( gif->fd );
		read( gif->fd, &cw, 1 );
		read( gif->fd, &ch, 1 );
		read( gif->fd, &fg, 1  );
		read( gif->fd, &bg, 1 );
		sub_block = gif->fd.Pos;
		gif->plain_text( gif, tx, ty, tw, th, cw, ch, fg, bg );
		gif->fd.Pos = sub_block;
	} else {
		/* Discard plain text metadata. */
		gif->fd.Pos += 13;
	}
	/* Discard plain text sub-blocks. */
	discard_sub_blocks( gif );
}

static void read_graphic_control_ext( CGiffDecodeData* gif )
{
	uint8_t rdit;

	/* Discard block size (always 0x04). */
	gif->fd.Pos += 1;
	read( gif->fd, &rdit, 1 );
	gif->gce.disposal = ( rdit >> 2 ) & 3;
	gif->gce.input = rdit & 2;
	gif->gce.transparency = rdit & 1;
	gif->gce.delay = read_num( gif->fd );
	read( gif->fd, &gif->gce.tindex, 1 );
	/* Skip block terminator. */
	gif->fd.Pos += 1;
}

static void read_comment_ext( CGiffDecodeData* gif )
{
	if( gif->comment ) {
		int sub_block = gif->fd.Pos;
		gif->comment( gif );
		gif->fd.Pos = sub_block;
	}
	/* Discard comment sub-blocks. */
	discard_sub_blocks( gif );
}

static void read_application_ext( CGiffDecodeData* gif )
{
	char app_id[8];
	char app_auth_code[3];

	/* Discard block size (always 0x0B). */
	gif->fd.Pos += 1;
	/* Application Identifier. */
	read( gif->fd, app_id, 8 );
	/* Application Authentication Code. */
	read( gif->fd, app_auth_code, 3 );
	if( !::strncmp( app_id, "NETSCAPE", sizeof( app_id ) ) ) {
		/* Discard block size (0x03) and constant byte (0x01). */
		gif->fd.Pos += 2;
		gif->loop_count = read_num( gif->fd  );
		/* Skip block terminator. */
		gif->fd.Pos += 1;
	} else if( gif->application ) {
		int sub_block = gif->fd.Pos;
		gif->application( gif, app_id, app_auth_code );
		gif->fd.Pos = sub_block;
		discard_sub_blocks( gif );
	} else {
		discard_sub_blocks( gif );
	}
}

static void read_ext( CGiffDecodeData* gif )
{
	uint8_t label;

	read( gif->fd, &label, 1 );
	switch( label ) {
		case 0x01:
			read_plain_text_ext( gif );
			break;
		case 0xF9:
			read_graphic_control_ext( gif );
			break;
		case 0xFE:
			read_comment_ext( gif );
			break;
		case 0xFF:
			read_application_ext( gif );
			break;
		default:
			throwGifError( "unknown extension" );
	}
}

static CGifTable new_table( int key_size )
{
	CGifTable table;
	int key;
	int init_bulk = max( 1 << ( key_size + 1 ), 0x100 );
	table.entries.IncreaseSize( init_bulk );
	table.bulk = init_bulk;
	table.nentries = ( 1 << key_size ) + 2;
	for( key = 0; key < ( 1 << key_size ); key++ ) {
		table.entries[key] = CGifEntry{ 1, 0xFFF, static_cast<uint8_t>( key ) };
	}
	return table;
}

/* Add table entry. Return value:
*  0 on success
*  +1 if key size must be incremented after this addition
*  -1 if could not realloc table */
static int add_entry( CGifTable* table, uint16_t length, uint16_t prefix, uint8_t suffix )
{
	if( table->nentries == table->bulk ) {
		table->bulk *= 2;
		table->entries.IncreaseSize( table->bulk );
	}
	table->entries[table->nentries] = CGifEntry{ length, prefix, suffix };
	table->nentries++;
	if( ( table->nentries & ( table->nentries - 1 ) ) == 0 ) {
		return 1;
	}
	return 0;
}

static uint16_t get_key( CGiffDecodeData* gif, int key_size, uint8_t* sub_len, uint8_t* shift, uint8_t* byte )
{
	int bits_read;
	int rpad;
	int frag_size;
	uint16_t key;

	key = 0;
	for( bits_read = 0; bits_read < key_size; bits_read += frag_size ) {
		rpad = ( *shift + bits_read ) % 8;
		if( rpad == 0 ) {
			/* Update byte. */
			if( *sub_len == 0 ) {
				read( gif->fd, sub_len, 1 ); /* Must be nonzero! */
			}
			read( gif->fd, byte, 1 );
			( *sub_len )--;
		}
		frag_size = min( key_size - bits_read, 8 - rpad );
		key |= ( (uint16_t) ( ( *byte ) >> rpad ) ) << bits_read;
	}
	/* Clear extra bits to the left. */
	key &= ( 1 << key_size ) - 1;
	*shift = ( *shift + key_size ) % 8;
	return key;
}

/* Compute output index of y-th input line, in frame of height h. */
static int interlaced_line_index( int h, int y )
{
	int p; /* number of lines in current pass */

	p = ( h - 1 ) / 8 + 1;
	if( y < p ) { /* pass 1 */
		return y * 8;
	}
	y -= p;
	p = ( h - 5 ) / 8 + 1;
	if( y < p ) { /* pass 2 */
		return y * 8 + 4;
	}
	y -= p;
	p = ( h - 3 ) / 4 + 1;
	if( y < p ) { /* pass 3 */
		return y * 4 + 2;
	}
	y -= p;
	/* pass 4 */
	return y * 2 + 1;
}

/* Decompress image pixels.
* Return 0 on success or -1 on out-of-memory (w.r.t. LZW code table). */
static int read_image_data( CGiffDecodeData* gif, int interlace )
{
	uint8_t sub_len, shift, byte;
	int init_key_size, key_size;
	int	table_is_full = 0;
	uint16_t str_len = 0;
	int frm_off, p, x, y;
	uint16_t key, clear, stop;
	int ret;
	CGifTable table;
	CGifEntry entry{};
	int start, end;

	read( gif->fd, &byte, 1 );
	key_size = (int) byte;
	start = gif->fd.Pos;
	discard_sub_blocks( gif );
	end = gif->fd.Pos;
	gif->fd.Pos = start;
	clear = 1 << key_size;
	stop = clear + 1;
	table = new_table( key_size );
	key_size++;
	init_key_size = key_size;
	sub_len = shift = 0;
	key = get_key( gif, key_size, &sub_len, &shift, &byte ); /* clear code */
	frm_off = 0;
	ret = 0;
	while( true ) {
		if( key == clear ) {
			key_size = init_key_size;
			table.nentries = ( 1 << ( key_size - 1 ) ) + 2;
			table_is_full = 0;
		} else if( !table_is_full ) {
			ret = add_entry( &table, str_len + 1, key, entry.suffix );
			if( ret == -1 ) {
				return -1;
			}
			if( table.nentries == 0x1000 ) {
				ret = 0;
				table_is_full = 1;
			}
		}
		key = get_key( gif, key_size, &sub_len, &shift, &byte );
		if( key == clear ) {
			continue;
		}
		if( key == stop ) {
			break;
		}
		if( ret == 1 ) {
			key_size++;
		}
		entry = table.entries[key];
		str_len = entry.length;
		while( true ) {
			p = frm_off + entry.length - 1;
			x = p % gif->fw;
			y = p / gif->fw;
			if( interlace ) {
				y = interlaced_line_index((int) gif->fh, y);
			}
			gif->frame[(gif->fy + y) * gif->width + gif->fx + x] = entry.suffix;
			if( entry.prefix == 0xFFF ) {
				break;
			} else {
				entry = table.entries[entry.prefix];
			}
		}
		frm_off += str_len;
		if( key < table.nentries - 1 && !table_is_full ) {
			table.entries[table.nentries - 1].suffix = entry.suffix;
		}
	}
	read( gif->fd, &sub_len, 1 ); /* Must be zero! */
	gif->fd.Pos = end;
	return 0;
}

/* Read image.
* Return 0 on success or -1 on out-of-memory (w.r.t. LZW code table). */
static int read_image( CGiffDecodeData* gif )
{
	uint8_t fisrz;
	int interlace;

	/* Image Descriptor. */
	gif->fx = read_num( gif->fd );
	gif->fy = read_num( gif->fd );
	gif->fw = read_num( gif->fd );
	gif->fh = read_num( gif->fd );
	read( gif->fd, &fisrz, 1 );
	interlace = fisrz & 0x40;
	/* Ignore Sort Flag. */
	/* Local Color Table? */
	if( fisrz & 0x80 ) {
		/* Read LCT */
		gif->lct->size = 1 << ( ( fisrz & 0x07 ) + 1 );
		read( gif->fd, gif->lct->colors, 3 * gif->lct->size );
		gif->palette = gif->lct;
	} else
		gif->palette = gif->gct;
	/* Image Data. */
	return read_image_data( gif, interlace );
}

static void render_frame_rect( CGiffDecodeData* gif, uint8_t* buffer, CDynamicBitSet<>& transparencyMask )
{
	int i, j, k;
	uint8_t index, *color;
	i = gif->fy * gif->width + gif->fx;
	for( j = 0; j < gif->fh; j++ ) {
		for( k = 0; k < gif->fw; k++ ) {
			const auto y = gif->fy + j;
			const auto x = gif->fx + k;
			const auto framePos = y * gif->width + x;
			index = gif->frame[framePos];
			if( !gif->gce.transparency || index != gif->gce.tindex ) {
				color = &gif->palette->colors[index*3];
				::memcpy( &buffer[( i + k ) * 3], color, 3 );
				transparencyMask -= framePos;
			}
		}
		i += gif->width;
	}
}

static void makeFrameTransparent( CGiffDecodeData* gif )
{
	for( int yDelta = 0; yDelta < gif->fh; yDelta++ ) {
		for( int xDelta = 0; xDelta < gif->fw; xDelta++ ) {
			const auto y = gif->fy + yDelta;
			const auto x = gif->fx + xDelta;
			const auto framePos = y * gif->width + x;
			gif->TransparencyMask |= framePos;
		}
	}
}

static void dispose( CGiffDecodeData* gif )
{
	switch( gif->gce.disposal ) {
		case 1:	// Do not dispose. Render the frame on the canvas.
			render_frame_rect( gif, gif->canvas, gif->TransparencyMask );
			break;
		case 2: // Restore to background color. The background is assumed to be transparent.
			makeFrameTransparent( gif );
			break;
		case 3: // Restore to previous. Leave the canvas as is and discard the frame.
			break;
		default:
			break;
	}
}

/* Return 1 if got a frame; 0 if got GIF trailer; -1 if error. */
int gd_get_frame( CGiffDecodeData* gif )
{
	char sep;

	dispose( gif );
	read( gif->fd, &sep, 1 );
	while( sep != ',' ) {
		if( sep == ';' ) {
			return 0;
		}
		if( sep == '!' ) {
			read_ext( gif );
		} else {
			return -1;
		}
		read( gif->fd, &sep, 1 );
	}
	if( read_image( gif ) == -1 ) {
		return -1;
	}
	return 1;
}

void gd_render_frame( CGiffDecodeData* gif, uint8_t* buffer, CDynamicBitSet<>& transparencyMask )
{
	auto& srcStorage = gif->TransparencyMask.GetStorage().GetStorage();
	auto& destStorage = transparencyMask.GetStorage().GetStorage();
	::memcpy( destStorage.Ptr(), srcStorage.Ptr(), destStorage.Size() * sizeof( DWORD ) );
	::memcpy( buffer, gif->canvas, gif->width * gif->height * 3 );
	render_frame_rect( gif, buffer, transparencyMask );
}

void gd_rewind( CGiffDecodeData* gif )
{
	gif->fd.Pos = gif->anim_start;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

#endif // RELIB_NO_IMAGELIB