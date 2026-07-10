#pragma once

#include <assert.h>
#include <stdbool.h>
#include <string.h>

// clang-format off
#include <_wingdi.h>
#include <_utils.h>
// clang-format on

// a struct representing a windows bitmap
typedef struct {
        fhead          fileheader;
        infhead        infoheader;
        rgbq*          pixels; // this points to the start of pixels in the file buffer i.e (buffer + 54)
        // pixels IS NOT A SEPARATE BUFFER, IT IS JUST A REFERENCE TO A BYTE FEW STRIDES (54 BYTES) INTO THE ACTUAL BYTES BUFFER
        unsigned char* buffer; // this will point to the original file buffer, this is the one that needs deallocation!
} bitmap;

// order of pixels in the BMP pixel buffer
typedef enum { TOPDOWN, BOTTOMUP } BITMAP_PIXEL_ORDERING;

// types of compressions used in BMP files
typedef enum { RGB, RLE8, RLE4, BITFIELDS, UNKNOWN } BITMAP_COMPRESSION_KIND;

static const unsigned short START_TAG_LE = L'M' << 8 | L'B';

static inline fhead fileheader(const unsigned char* const restrict imstream, const unsigned size) {
    assert(size >= sizeof(fhead));
    fhead header = { 0 };

    // bitmap files store a tag as 'B', followed by 'M', i.e 0x424D as an unsigned 16 bit integer,
    // when we dereference this 16 bits as an unsigned 16 bit integer on LE machines, the byte order will get swapped i.e the two bytes will be read as 'M', 'B'
    if (*((unsigned short*) (imstream)) != START_TAG_LE) {
        fprintf(
            stderr,
            "Error in function %s in file %s at line %d, file appears not to be a Windows bitmap\n",
            __FUNCTION__,
            __FILE__,
            __LINE__
        );
        free(imstream);
        return header;
    }

    header.type    = START_TAG_LE;
    header.size    = *(unsigned*) (imstream + 2);
    header.offbits = *(unsigned*) (imstream + 10);

    return header;
}

static inline infhead infoheader(const unsigned char* const imstream, const unsigned size) {
    assert(size >= (sizeof(fhead) + sizeof(infhead)));
    infhead header = { 0 };

    if (*((unsigned*) (imstream + 14U)) > 40U) {
        fprintf(
            stderr,
            "Error in function %s in file %s at line %d, the bitmap contains an unparsable file info header\n",
            __FUNCTION__,
            __FILE__,
            __LINE__
        );
        free(imstream);
        return header;
    }

    header.size        = *((unsigned*) (imstream + 14U));
    header.width       = *((int*) (imstream + 18U));
    header.height      = *((int*) (imstream + 22U));
    header.planes      = *((unsigned short*) (imstream + 26U));
    header.nbits       = *((unsigned short*) (imstream + 28U));
    header.compression = *((unsigned*) (imstream + 30U));
    header.imagesize   = *((unsigned*) (imstream + 34U));
    header.ppm_x       = *((int*) (imstream + 38U));
    header.ppm_y       = *((int*) (imstream + 42U));
    header.used_clrs   = *((unsigned*) (imstream + 46U));
    header.imp_clrs    = *((unsigned*) (imstream + 50U));

    return header;
}

static inline BITMAP_PIXEL_ORDERING pixelorder(const infhead* const header) { return (header->height >= 0) ? BOTTOMUP : TOPDOWN; }

// reads in a bmp file from disk and deserializes it into a bitmap_t struct
static inline bitmap bmpread(const char* const filepath) {
    long   size  = 0;
    bitmap image = { 0 }; // will be used as an empty placeholder for premature returns until members are properly assigned

    const unsigned char* const buffer = imopen(filepath, &size);
    if (!buffer) return image; // imopen will do the error reporting, so just exiting the function is enough

    const fhead file_header = fileheader(buffer, size);
    if (!file_header.size) return image;
    // again fileheader will report errors and free the buffer, if the predicate isn't satisified, just exit the routine

    const infhead info_header = infoheader(buffer, size);
    if (!info_header.size) return image; // error reporting and resource cleanup are handled by infoheader

    image.fileheader = file_header;
    image.infoheader = info_header;
    image.buffer     = buffer;
    image.pixels     = (rgbq*) (buffer + 54);

    return image;
}

// use this to cleanup a bitmap_t after its use
static inline void bmpclose(bitmap* const image) {
    free(image->buffer);
    memset(image, 0, sizeof(bitmap));
}
