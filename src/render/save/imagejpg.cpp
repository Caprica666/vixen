
#define XMD_H
#define HAVE_PROTOTYPES

extern "C"
{
#include "libjpg\jpeglib.h"
#include "libjpg\jerror.h"
};

#include "psm.h"

#define INPUT_BUF_SIZE	1024
#define	OUTPUT_BUF_SIZE	1024

class jpeg_source : public jpeg_source_mgr
{
public:
	PSStream*	file_handle;	/* source stream */
	JOCTET*		buffer;			/* start of buffer */
};

class jpeg_dest : public jpeg_destination_mgr
{
public:
	PSStream*	file_handle;	/* destination stream */
	JOCTET*		buffer;			/* start of buffer */
};


static void jpeg_error_exit (j_common_ptr cinfo)
{
	(*cinfo->err->output_message)(cinfo);

	jpeg_destroy(cinfo);

//	throw FIF_JPEG;
}

static void jpeg_output_message (j_common_ptr cinfo)
{
	char buffer[JMSG_LENGTH_MAX];

	(*cinfo->err->format_message)(cinfo, buffer);
}

static void init_source(j_decompress_ptr cinfo)
{
	jpeg_source* src = (jpeg_source*) cinfo->src;

	/* We reset the empty-input-file flag for each image,
 	 * but we don't clear the input buffer.
	 * This is correct behavior for reading a series of images from one source.
	*/
//	src->start_of_file = TRUE;
}

/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of throw() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */
static boolean fill_input_buffer(j_decompress_ptr cinfo)
{
	jpeg_source* src = (jpeg_source*) cinfo->src;

	size_t nbytes = src->file_handle->Read((char*) src->buffer, INPUT_BUF_SIZE);

	if (nbytes <= 0)
	{
		cinfo->err->msg_code = JWRN_JPEG_EOF;
		cinfo->err->emit_message((j_common_ptr) cinfo, -1);

		/* Insert a fake EOI marker */
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}
	src->next_input_byte = src->buffer;
	src->bytes_in_buffer = nbytes;
	return TRUE;
}

static boolean empty_output_buffer(j_compress_ptr cinfo)
{
	jpeg_dest* dest = (jpeg_dest*) cinfo->dest;

	if (dest->file_handle->Write((char*) dest->buffer, OUTPUT_BUF_SIZE) != OUTPUT_BUF_SIZE)
//		throw(cinfo, JERR_FILE_WRITE);
		return FALSE;
	dest->next_output_byte = dest->buffer;
	dest->free_in_buffer = OUTPUT_BUF_SIZE;
	return TRUE;
}

/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */
static void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	jpeg_source* src = (jpeg_source*) cinfo->src;

	/* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
	 * any trouble anyway --- large skips are infrequent.
	*/

	if (num_bytes > 0)
	{
		while (num_bytes > (long) src->bytes_in_buffer)
		{
		  num_bytes -= (long) src->bytes_in_buffer;

		  (void) fill_input_buffer(cinfo);

		  /* note we assume that fill_input_buffer will never return FALSE,
		   * so suspension need not be handled.
		   */
		}
		src->next_input_byte += (size_t) num_bytes;
		src->bytes_in_buffer -= (size_t) num_bytes;
	}
}

/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */
static void term_source (j_decompress_ptr cinfo) {
  /* no work necessary here */
}

static void term_destination (j_compress_ptr cinfo)
{
	jpeg_dest* dest = (jpeg_dest*) cinfo->dest;
	size_t datacount = OUTPUT_BUF_SIZE - dest->free_in_buffer;

	/* Write any data remaining in the buffer */
	if (datacount > 0)
	{
		if (dest->file_handle->Write((char*) dest->buffer, datacount) != datacount)
//		  throw(cinfo, JERR_FILE_WRITE);
			return;
	}
}


// initialize the jpeg pointer struct with pointers to functions
static void jpeg_init_funcs(jpeg_source* src, PSStream* filehandle)
{
	src->file_handle = filehandle;
	src->init_source = init_source;
	src->fill_input_buffer = fill_input_buffer;
	src->skip_input_data = skip_input_data;
	src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->term_source = term_source;
	src->bytes_in_buffer = 0;			/* forces fill_input_buffer on first read */
	src->next_input_byte = NULL;		/* until buffer loaded */
}

bool PSImageReader::JPGRead(PSBitmap* bmap, PSStream* stream)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	void*	dib = NULL;

	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit     = jpeg_error_exit;
	jerr.output_message = jpeg_output_message;
/*
 * allocate and initialize JPEG decompression object
 * allocate memory for the buffer. is released automatically in the end
 */
	jpeg_create_decompress(&cinfo);
	jpeg_source* src = (jpeg_source*) (cinfo.mem->alloc_small)
		((j_common_ptr) &cinfo, JPOOL_PERMANENT, sizeof(jpeg_source));
	cinfo.src = src;
	src->buffer = (JOCTET *) (cinfo.mem->alloc_small)
		((j_common_ptr) &cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
	jpeg_init_funcs(src, stream);
/*
 * read handle parameters with jpeg_read_header()
 * and set parameters for decompression
 */
	jpeg_read_header(&cinfo, TRUE);
	cinfo.dct_method          = JDCT_IFAST;
	cinfo.do_fancy_upsampling = FALSE;
/*
 * compute bytes per scanline
 */
	Pitch = cinfo.image_width * cinfo.num_components;
	Pitch = (Pitch + 3) & ~3;
/*
 * start decompressor and compress each scan line individually
 */
	unsigned char* bits = (unsigned char*) malloc(cinfo.image_height * Pitch);

	jpeg_start_decompress(&cinfo);
	while (cinfo.output_scanline < cinfo.output_height)
	{
//		JSAMPROW b = bits + (cinfo.output_height - cinfo.output_scanline - 1) * Pitch;
		JSAMPROW b = bits + (cinfo.output_scanline) * Pitch;

		jpeg_read_scanlines(&cinfo, &b, 1);
	}
/*
 * finish decompression and release resources
 */
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
    stream->Close();
	bmap->Lock();
	bmap->Format = IMAGE_HasColor;
	bmap->Depth = cinfo.num_components * 8;
	bmap->Data = bits;
	bmap->Width = cinfo.image_width;
	bmap->Height = cinfo.image_height;
	bmap->Type = IMAGE_DXBitmap;
	bmap->Unlock();
	return true;
}

#if 0
bool PSImageReader::JPGWrite(FILE* handle, int quality)
{
	if ((m_pDib == NULL) || (handle == NULL))
		return false;

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	BITMAPINFOHEADER *pInfoHeader = (BITMAPINFOHEADER*) ((unsigned char*) m_pDib) + sizeof(BITMAPFILEHEADER);
	int		width = pInfoHeader->biWidth;
	int		height = pInfoHeader->biHeight;
	int		bpp = pInfoHeader->biBitCount;	

	if ((bpp != 24) && (bpp != 8))
		return false;
/*
 * allocate and initialize JPEG compression object
 */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit     = jpeg_error_exit;
	jerr.output_message = jpeg_output_message;
	jpeg_create_compress(&cinfo);
/*
 * specify data destination file
 */
	jpeg_dest* dest = (jpeg_dest*) (cinfo.mem->alloc_small)
			((j_common_ptr) &cinfo, JPOOL_PERMANENT, sizeof(jpeg_dest));
	cinfo.dest = dest;
	dest->next_output_byte = dest->buffer;
	dest->free_in_buffer = OUTPUT_BUF_SIZE;
	dest->file_handle = NULL;
/* 
 * set parameters for compression
 */
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.in_color_space = JCS_RGB;
	cinfo.input_components = 3;
	jpeg_set_defaults(&cinfo);

/*
 * Set JFIF density parameters from the DIB data
 */
	cinfo.X_density = (WORD)(pInfoHeader->biXPelsPerMeter / 100.0F + 0.5);
	cinfo.Y_density = (WORD)(pInfoHeader->biYPelsPerMeter / 100.0F + 0.5);
	cinfo.density_unit = 2;	// dots / cm

	jpeg_set_quality(&cinfo, quality, TRUE); /* limit to baseline-JPEG values */
/*
 * Compress each scan line individually
 */
	RGBQUAD *palette = GetPalette();
	unsigned char* bits = (unsigned char*) (palette + 256);

 	jpeg_start_compress(&cinfo, TRUE);
	if (bpp > 8)				// 24-bit or 8-bit greyscale images
		while (cinfo.next_scanline < cinfo.image_height)
		{
			JSAMPROW b = bits + (cinfo.next_scanline - 1) * m_Pitch;
			jpeg_write_scanlines(&cinfo, &b, 1);
		}
	else						// 8-bit palettized images are converted to 24-bit images
	{
		BYTE*	target = (BYTE*) malloc(cinfo.image_width * 3);
		while (cinfo.next_scanline < cinfo.image_height)
		{
			BYTE* source = bits + m_Pitch * (height - cinfo.next_scanline - 1);
			BYTE* p = target;
			for (int cols = 0; cols < width; cols++)
			{
				p[0] = palette[source[cols]].rgbBlue;
				p[1] = palette[source[cols]].rgbGreen;
				p[2] = palette[source[cols]].rgbRed;
				p += 3;
			}
			jpeg_write_scanlines(&cinfo, &target, 1);
		}
		free(target);
	}
/* 
 * clean up and exit
 */
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	stream->Close();
	return TRUE;
}

#endif


