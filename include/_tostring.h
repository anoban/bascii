#pragma once

#include <stdbool.h>
#include <stddef.h>

// clang-format off
#include <_bitmap.h>
#include <_utils.h>
// clang-format on

//----------------------------------
//    PLACE FOR CUSTOMIZATIONS
//----------------------------------

#define CONSOLE_WIDTH  140
#define CONSOLE_WIDTHR ((double) (CONSOLE_WIDTH))

//----------------------------------

static inline char* to_raw_string(
    const bitmap* const image, char (*mapper)(const rgbq* const, const char* const, unsigned), const char* const palette, unsigned psize
) {
    if (pixelorder(&image->infoheader) == TOPDOWN) {
        fprintf(
            stderr,
            "Error in function %s in file %s at line %d, bitmaps with top-down pixel orders are not supported!\n",
            __FUNCTION__,
            __FILE__,
            __LINE__
        );
        return NULL;
    }

    const long long npixels = (long long) (image->infoheader.height) * image->infoheader.width; // total pixels in the image
    const long long nchars /* 1 char for each pixel + 1 additional character for LF ('\n') at the end of each scanline */ =
        npixels + 1LL * image->infoheader.height;

    char* const buffer = malloc(nchars + 1); // and the +1 is for the NULL terminator
    if (!buffer) {
        fprintf(stderr, "Error in function %s in file %s at line %d, memory allocation failed!\n", __FUNCTION__, __FILE__, __LINE__);
        return NULL;
    }

    // pixels are organized in rows from bottom to top and, within each row, from left to right, each row is called a "scan line".
    // if the image height is given as a negative number, then the rows are ordered from top to bottom
    // in most contemporary bitmap images, the pixel ordering is  bottom up

    // (pixel at the top left corner of the image)
    //                                            10 11 12 13 14 15 16 17 18 19 <-- this will be the last pixel (pixel at the bottom right corner of the image)
    //                                            .............................
    //                                            .............................
    //                                            .............................
    // this is the first pixel in the buffer -->  00 01 02 03 04 05 06 07 08 09
    // (pixel at the top left corner of the image)

    long long caret = 0;

    // traverse up along the height, for each row, starting with the last row
    for (long long row = image->infoheader.height - 1LL; row >= 0; --row) {
        // traverse left to right inside "scan lines"
        for (long long col = 0; col < image->infoheader.width; ++col) // for each pixel in the row,
            buffer[caret++] = mapper(&image->pixels[row * image->infoheader.width + col], palette, psize);
        buffer[caret++] = L'\n'; // at the end of each scanline, append an LF!
    }

    buffer[caret] = 0; // null termination of the string

    if (caret != nchars) {
        fprintf(
            stderr,
            "Error in function %s in file %s at line %d, mismatch between number of characters mapped (%lld) and number of characters estimated to be mapped (%lld)\n",
            __FUNCTION__,
            __FILE__,
            __LINE__,
            caret,
            nchars
        );
        free(buffer);
        return NULL;
    }

    return buffer;
}

static inline char* to_downscaled_string(
    const bitmap* const image, char (*mapper)(float, float, float, const char* const, unsigned), const char* const palette, unsigned psize
) {
    // generate the character buffer after downscaling the image such that the ASCII representation will fit the terminal width (~142 chars)
    // downscaling is solely dependent on the image width, and a proportionate scaling factor will be used to downscale the image vertically too
    // downscaling needs to be done in square pixel blocks which will be represented by a single ASCII character

    if (pixelorder(&image->infoheader) == TOPDOWN) {
        fprintf(
            stderr,
            "Error in function %s in file %s at line %d, bitmaps with top-down pixel orders are not supported!\n",
            __FUNCTION__,
            __FILE__,
            __LINE__
        );
        return NULL;
    }

    const long long block_d                    = ceil(image->infoheader.width / CONSOLE_WIDTHR); // dimension of an individual square block
    const long long blocksize                  = block_d * block_d;                              // number of pixels in a square block

    // if the number of pixels in a row is not divisible without remainders by our block width, the last column of blocks will be incomplete
    // i.e. width smaller than the precalculated block width
    const long long incomplete_blocksize_right = // number of pixels in each block in the rightmost column of incomplete blocks.
        // width of the image - (number of complete blocks * block dimension) will give the residual pixels along rows
        // multiply that by block height, we'll get the number of pixels in the incomplete block
        (image->infoheader.width - // signed integer divisions truncate towards 0 (flooring)
         (image->infoheader.width / block_d) /* deliberate integer division to only get the count of complete blocks */ * block_d) *
        block_d;

    if (incomplete_blocksize_right >= blocksize) { // shouldn't be - indicates a logic error
        fprintf(
            stderr,
            "Error in function %s in file %s at line %d, size of an incomplete block (%lld) cannot be equal to or bigger than the size of a complete block (%lld)!\n",
            __FUNCTION__,
            __FILE__,
            __LINE__,
            incomplete_blocksize_right,
            blocksize
        );
        return NULL;
    }

    // similarly, when the image height is indivisible without remainders by the block height, the last row of blocks (upper most in the image)
    // will be incomplete i.e. with a height shorter than the block height
    // to calculate this, we reapply the same logic, this time, to the vertical axis
    const long long incomplete_blocksize_bottom = (image->infoheader.height - (image->infoheader.height / block_d) * block_d) * block_d;

    if (incomplete_blocksize_bottom >= blocksize) {
        fprintf(
            stderr,
            "Error in function %s in file %s at line %d, size of an incomplete block (%lld) cannot be equal to or bigger than the size of a complete block (%lld)!\n",
            __FUNCTION__,
            __FILE__,
            __LINE__,
            incomplete_blocksize_bottom,
            blocksize
        );
        return NULL;
    }

    // number of total blocks along rows (including incomplete blocks) - hence the round up with ceil()
    const long long hblocks_total = ceil((double) image->infoheader.width / block_d);
    // number of total blocks along columns (including incomplete blocks)
    const long long vblocks_total = ceil((double) image->infoheader.height / block_d);

    // we have to compute the average R, G & B values for all pixels inside each pixel block and use the average to represent that block as a character.
    // one char in our buffer will have to represent (block_w x block_h) number of RGBQUADs
    const long long nchars        = vblocks_total * (hblocks_total + 1) + 1; // saving one char for LF, the +1 is for the NULL terminator

    char* const buffer            = malloc(nchars);
    if (!buffer) {
        fprintf(stderr, "Error in function %s in file %s at line %d memory allocation failed!\n", __FUNCTION__, __FILE__, __LINE__);
        return NULL;
    }

    double    avgb = 0.0, avgg = 0.0, avgr = 0.0; // per block averages of the rgbBlue, rgbGreen and rgbRed values
    long long caret = 0, offset = 0, col = 0, row = 0;

    // true if the image width is not divisible by CONSOLE_WIDTH without remainders
    const bool hblocks_incomplete = image->infoheader.width % CONSOLE_WIDTH;
    // true if the image height is not divisible by block_d without remainders
    const bool vblocks_incomplete = image->infoheader.height % block_d;

    fprintf(stderr, "Width :: %d, Height :: %d\n", image->infoheader.width, image->infoheader.height);
    fprintf(stderr, "Size of the square block :: %6lld\n", block_d);
    fprintf(stderr, "Number of blocks along the x axis :: %6lld\n", hblocks_total);
    fprintf(
        stderr,
        "Dimension of the incomplete block along the x axis (w, h) :: (%3lld, %3lld)\n",
        image->infoheader.width - (image->infoheader.width / block_d) * block_d,
        hblocks_total
    );
    fprintf(stderr, "Number of blocks along the y axis :: %6lld\n", nblocks_h);
    fprintf(
        stderr,
        "Dimension of the incomplete block along the y axis (w, h) :: (%3lld, %3lld)\n",
        nblocks_w,
        image->_infoheader.biHeight - (image->_infoheader.biHeight / block_d) * block_d
    );

    unsigned long long full = 0, incomplete = 0, count = 0; // NOLINT(readability-isolate-declaration)

    // row = image->_infoheader.biHeight will get us to the last pixel of the first (last in the buffer) scanline with (r * image->_infoheader.biWidth)
    // hence, row = image->_infoheader.biHeight - 1 so we can traverse pixels in the first scanline with (r * image->_infoheader.biWidth) + c
    for (row = image->infoheader.height - 1; row >= (block_d - 1); row -= block_d) { // start the traversal at the bottom most scan line
                                                                                     // wprintf_s(L"row = %lld\n", row);
        for (col = 0; col <= image->infoheader.width - block_d; col += block_d) {    // traverse left to right in scan lines
            // wprintf_s(L"row = %lld, col = %lld\n", row, col);

            for (long long r = row; r > row - block_d; --r) { // deal with blocks
                for (long long c = col; c < col + block_d; ++c) {
                    offset  = (r * image->infoheader.width) + c;
                    avgb   += image->pixels[offset].b;
                    avgg   += image->pixels[offset].g;
                    avgr   += image->pixels[offset].r;

                    __debug(count++);
                }
            }

            __debug(full++);
            assert(count == block_d * block_d);
            __debug(count = 0);

            avgb /= blocksize;
            avgg /= blocksize;
            avgr /= blocksize;

            assert(avgb <= 255.00 && avgg <= 255.00 && avgr <= 255.00);

            buffer[caret++] = mapper(avgb, avgg, avgr, palette, psize);
            avgb = avgg = avgr = 0.000;
        }

        if (hblocks_incomplete) { // if there are partially filled blocks at the end of this row of blocks,

            for (long long r = row; r > row - block_d; --r) {
                // shift the column delimiter backward by one block, to the end of the last complete block
                for (long long c = col; c < image->infoheader.width; ++c) { // start from the end of the last complete block
                    offset  = (r * image->infoheader.width) + c;
                    avgb   += image->pixels[offset].b;
                    avgg   += image->pixels[offset].g;
                    avgr   += image->pixels[offset].r;

                    __debug(count++);
                }
            }

            __debug(incomplete++);
            assert(count == incomplete_blocksize_right); // fails
            __debug(count = 0);

            avgb /= incomplete_blocksize_right;
            avgg /= incomplete_blocksize_right;
            avgr /= incomplete_blocksize_right;

            assert(avgb <= 255.00 && avgg <= 255.00 && avgr <= 255.00);

            buffer[caret++] = blockmap(avgb, avgg, avgr);
            avgb = avgg = avgr = 0.000; // reset the block averages
        }

        buffer[caret++] = L'\n';
        // buffer[caret++] = L'\r';
    }

    __printf_debug("%5llu complete blocks have been processed!\n", full);
    __printf_debug("%5llu incomplete blocks at the right edge have been processed\n", incomplete);
    assert(row < block_d);
    __debug(incomplete = 0);

    if (vblocks_incomplete) { // process the last incomplete row of pixel blocks here,

        for (col = 0; col < image->infoheader.width; col += block_d) { // col must be 0 at the start of this loop

            for (long long r = row; r >= 0; --r) {                // r delimits the start row of the block being defined
                for (long long c = col; c < col + block_d; ++c) { // c delimits the start column of the block being defined
                    offset  = (r * image->infoheader.width) + c;
                    avgb   += image->pixels[offset].b;
                    avgg   += image->pixels[offset].g;
                    avgr   += image->pixels[offset].r;
                }
            }

            avgb /= incomplete_blocksize_bottom;
            avgg /= incomplete_blocksize_bottom;
            avgr /= incomplete_blocksize_bottom;

            __debug(incomplete++);

            // if (!(blockavg_blue <= 255.00 && blockavg_green <= 255.00 && blockavg_red <= 255.00))
            //     wprintf_s(L"Average (BGR) = (%.4f, %.4f, %.4f)\n", blockavg_blue, blockavg_green, blockavg_red);

            assert(avgb <= 255.00 && avgg <= 255.00 && avgr <= 255.00);
            buffer[caret++] = blockmap(avgb, avgg, avgr);
            avgb = avgg = avgr = 0.000; // reset the block averages
        }

        buffer[caret++] = '\n';
        // buffer[caret++] = '\r';
    }

    buffer[caret++] = 0; // using the last byte as null terminator

    // now caret == nwchars, so an attempt to write at caret will now raise an access violation exception or a heap corruption error

    __printf_debug("%5llu incomplete blocks at the bottom have been processed\n", incomplete);
    __printf_debug("caret :: %lld, nwchars :: %lld\n", caret, nwchars);
    assert(caret == nchars);

    return buffer;
}

// an image width dependent dispatcher for to_raw_string and to_downscaled_string, that actually do the heavy lifting
static inline char* to_string(const bitmap* const image) {
    if (image->infoheader.width <= CONSOLE_WIDTH) return to_raw_string(image);
    return to_downscaled_string(image);
}
