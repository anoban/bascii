#pragma once

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/stat.h>

// clang-format off
#include <_wingdi.h>
// clang-format on

static inline unsigned char* imopen(const char* const fpath, long* const nreadbytes) {
    *nreadbytes             = 0;
    unsigned char* buffer   = NULL;
    struct stat    filestat = {};
    long           nbytes   = 0;

    const int fdesc         = open(fpath, O_RDONLY);
    if (fdesc == -1) { // if open() failed, the return value will be -1
        fprintf(stderr, "Call to open() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
        return NULL;
    }

    if (fstat(fdesc, &filestat)) { // if succeeds, 0 is returned, -1 if fails
        fprintf(stderr, "Call to fstat() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
        goto CLOSE_AND_RETURN;
    }

    if (!(buffer = malloc(filestat.st_size))) { // caller is responsible for freeing this buffer
        fprintf(stderr, "Call to new() failed inside %s at line %d!\n", __FUNCTION__, __LINE__);
        goto CLOSE_AND_RETURN;
    }

    if ((nbytes = read(fdesc, buffer, filestat.st_size)) != -1) {
        *nreadbytes = nbytes;
        assert(nbytes == filestat.st_size); // double checking
    } else {
        fprintf(stderr, "Call to read() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
        free(buffer);
        buffer = NULL;
    }
    // then, fall through the CLOSE_AND_RETURN label

CLOSE_AND_RETURN:
    // close() returns 0 on success and -1 on failure
    if (close(fdesc)) fprintf(stderr, "Call to close() failed inside %s at line %d!; errno %d\n", __FUNCTION__, __LINE__, errno);
    return buffer;
}

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

static inline fhead fileheader(const unsigned char* const buffer, const unsigned size) {
    fhead header = { 0 };

    if ((size < sizeof(fhead)) || !buffer) { // if the buffer is NULL or it's smaller than the size of the file header
        fprintf(
            stderr, "Error in function %s in file %s at line %d, invalid buffer with size %u\n", __FUNCTION__, __FILE__, __LINE__, size
        );
        return header;
    }

    // bitmap files store a tag as 'B', followed by 'M', i.e 0x424D as an unsigned 16 bit integer,
    // when we dereference this 16 bits as an unsigned 16 bit integer on LE machines, the byte order will get swapped i.e the two bytes will be read as 'M', 'B'
    if (*((unsigned short*) (buffer)) != START_TAG_LE) {
        fprintf(
            stderr,
            "Error in function %s in file %s at line %d, file appears not to be a Windows bitmap\n",
            __FUNCTION__,
            __FILE__,
            __LINE__
        );
        free(buffer);
        return header;
    }

    header.type    = START_TAG_LE;
    header.size    = *(unsigned*) (buffer + 2);
    header.offbits = *(unsigned*) (buffer + 10);

    return header;
}

static inline infhead infoheader(const unsigned char* const buffer, const unsigned size) {
    infhead header = { 0 };

    if ((size < (sizeof(fhead) + sizeof(infhead))) || !buffer) {
        fprintf(
            stderr, "Error in function %s in file %s at line %d, invalid buffer with size %u\n", __FUNCTION__, __FILE__, __LINE__, size
        );
        return header;
    }

    if (*((unsigned*) (buffer + 14U)) > 40U) {
        fprintf(
            stderr,
            "Error in function %s in file %s at line %d, the bitmap contains an unparsable file info header\n",
            __FUNCTION__,
            __FILE__,
            __LINE__
        );
        free(buffer);
        return header;
    }

    header.size        = *((unsigned*) (buffer + 14U));
    header.width       = *((int*) (buffer + 18U));
    header.height      = *((int*) (buffer + 22U));
    header.planes      = *((unsigned short*) (buffer + 26U));
    header.nbits       = *((unsigned short*) (buffer + 28U));
    header.compression = *((unsigned*) (buffer + 30U));
    header.imagesize   = *((unsigned*) (buffer + 34U));
    header.ppm_x       = *((int*) (buffer + 38U));
    header.ppm_y       = *((int*) (buffer + 42U));
    header.used_clrs   = *((unsigned*) (buffer + 46U));
    header.imp_clrs    = *((unsigned*) (buffer + 50U));

    return header;
}

static inline BITMAP_PIXEL_ORDERING __attribute__((always_inline)) pixelorder(const infhead* const header) {
    return (header->height >= 0) ? BOTTOMUP : TOPDOWN;
}

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
static inline void __attribute__((always_inline)) bmpclose(bitmap* const image) {
    free(image->buffer);
    memset(image, 0, sizeof(bitmap));
}
