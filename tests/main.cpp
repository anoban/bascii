#include <ctime>

#ifndef __VERBOSE_OUTPUTS
    #define __VERBOSE_OUTPUTS 1
#endif

#ifndef __TEST__
    #define __TEST__ 1
#endif

// clang-format off
#include <gtest/gtest.h>
namespace bascii {
#include <tostring.h>
}
// clang-format on

static constexpr rgbq min { .b = 0x00, .g = 0x00, .r = 0x00, ._ = 0xFF };
static constexpr rgbq mid { .b = 0x80, .g = 0x80, .r = 0x80, ._ = 0xFF };
static constexpr rgbq max { .b = 0xFF, .g = 0xFF, .r = 0xFF, ._ = 0xFF };

// a 300 byte chunk extracted from a real BMP file, for testing
static constexpr unsigned char dummybmp[] {
    66, 77,  54, 129, 21, 0,   0,   0,  0,  0,   54,  0,  0,  0,   40, 0, 0,  0,   222, 2, 0,  0,   224, 1, 0,  0,   1, 0, 32, 0,   0, 0,
    0,  0,   0,  0,   0,  0,   196, 14, 0,  0,   196, 14, 0,  0,   0,  0, 0,  0,   0,   0, 0,  0,   2,   2, 8,  255, 2, 2, 8,  255, 1, 1,
    7,  255, 1,  1,   7,  255, 0,   0,  6,  255, 0,   1,  5,  255, 0,  1, 5,  255, 0,   1, 5,  255, 0,   1, 5,  255, 2, 1, 5,  255, 7, 4,
    6,  255, 8,  5,   7,  255, 7,   3,  8,  255, 8,   4,  9,  255, 8,  4, 9,  255, 8,   4, 9,  255, 7,   3, 8,  255, 6, 4, 8,  255, 5, 4,
    8,  255, 5,  4,   8,  255, 5,   4,  8,  255, 5,   4,  8,  255, 5,  3, 9,  255, 5,   3, 9,  255, 5,   3, 9,  255, 5, 3, 9,  255, 5, 3,
    9,  255, 5,  3,   9,  255, 5,   3,  9,  255, 5,   3,  9,  255, 5,  3, 9,  255, 5,   3, 9,  255, 5,   3, 9,  255, 5, 3, 9,  255, 5, 3,
    9,  255, 5,  3,   9,  255, 5,   3,  9,  255, 5,   3,  9,  255, 5,  3, 9,  255, 5,   3, 9,  255, 5,   3, 9,  255, 5, 3, 9,  255, 5, 3,
    9,  255, 4,  3,   13, 255, 7,   6,  16, 255, 8,   7,  17, 255, 8,  6, 18, 255, 8,   6, 18, 255, 7,   7, 19, 255, 7, 7, 19, 255, 6, 6,
    18, 255, 6,  6,   18, 255, 6,   6,  18, 255, 6,   6,  18, 255, 6,  6, 18, 255, 7,   7, 19, 255, 7,   7, 19, 255, 7, 7, 19, 255, 7, 7,
    19, 255, 8,  8,   20, 255, 8,   8,  20, 255, 8,   8
};

static constexpr float RANDMAX { RAND_MAX + 2.0000 };
// the + 2.0000 is just for extra safety that we do not get too close to 1.000 when dividing rand() by RNDMAX

static constexpr unsigned long long PALETTE_LENGTHS[] { sizeof(PALETTE_MINIMAL), sizeof(PALETTE_BASE), sizeof(PALETTE_EXTENDED) };

// make sure that all the basic mappers don't compute off-bound offsets for palettes
TEST(basic_mappers, arithmetic) {
    rgbq test { 0x00, 0x00, 0x00, 0xFF };

    for (unsigned b = 0; b <= UCHAR_MAX; ++b) {
        for (unsigned g = 0; g <= UCHAR_MAX; ++g) {
            for (unsigned r = 0; r <= UCHAR_MAX; ++r) {
                test.b = b;
                test.g = g;
                test.r = r;

                ASSERT_NE(
                    ::memchr(PALETTE_BASE, bascii::arithmetic(&test, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)), NULL
                );
                ASSERT_NE(
                    ::memchr(PALETTE_MINIMAL, bascii::arithmetic(&test, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)), sizeof(PALETTE_MINIMAL)),
                    NULL
                );
                ASSERT_NE(
                    ::memchr(
                        PALETTE_EXTENDED, bascii::arithmetic(&test, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)), sizeof(PALETTE_EXTENDED)
                    ),
                    NULL
                );
            }
        }
    }
}

TEST(basic_mappers, weighted) {
    rgbq test { 0x00, 0x00, 0x00, 0xFF };

    for (unsigned b = 0; b <= UCHAR_MAX; ++b) {
        for (unsigned g = 0; g <= UCHAR_MAX; ++g) {
            for (unsigned r = 0; r <= UCHAR_MAX; ++r) {
                test.b = b;
                test.g = g;
                test.r = r;

                ASSERT_NE(::memchr(PALETTE_BASE, bascii::weighted(&test, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)), NULL);
                ASSERT_NE(
                    ::memchr(PALETTE_MINIMAL, bascii::weighted(&test, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)), sizeof(PALETTE_MINIMAL)),
                    NULL
                );
                ASSERT_NE(
                    ::memchr(
                        PALETTE_EXTENDED, bascii::weighted(&test, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)), sizeof(PALETTE_EXTENDED)
                    ),
                    NULL
                );
            }
        }
    }
}

TEST(basic_mappers, minmax) {
    rgbq test { 0x00, 0x00, 0x00, 0xFF };

    for (unsigned b = 0; b <= UCHAR_MAX; ++b) {
        for (unsigned g = 0; g <= UCHAR_MAX; ++g) {
            for (unsigned r = 0; r <= UCHAR_MAX; ++r) {
                test.b = b;
                test.g = g;
                test.r = r;

                ASSERT_NE(::memchr(PALETTE_BASE, bascii::minmax(&test, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)), NULL);
                ASSERT_NE(
                    ::memchr(PALETTE_MINIMAL, bascii::minmax(&test, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)), sizeof(PALETTE_MINIMAL)),
                    NULL
                );
                ASSERT_NE(
                    ::memchr(PALETTE_EXTENDED, bascii::minmax(&test, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)), sizeof(PALETTE_EXTENDED)),
                    NULL
                );
            }
        }
    }
}

TEST(basic_mappers, luminosity) {
    rgbq test { 0x00, 0x00, 0x00, 0xFF };

    for (unsigned b = 0; b <= UCHAR_MAX; ++b) {
        for (unsigned g = 0; g <= UCHAR_MAX; ++g) {
            for (unsigned r = 0; r <= UCHAR_MAX; ++r) {
                test.b = b;
                test.g = g;
                test.r = r;

                ASSERT_NE(
                    ::memchr(PALETTE_BASE, bascii::luminosity(&test, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)), NULL
                );
                ASSERT_NE(
                    ::memchr(PALETTE_MINIMAL, bascii::luminosity(&test, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)), sizeof(PALETTE_MINIMAL)),
                    NULL
                );
                ASSERT_NE(
                    ::memchr(
                        PALETTE_EXTENDED, bascii::luminosity(&test, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)), sizeof(PALETTE_EXTENDED)
                    ),
                    NULL
                );
            }
        }
    }
}

int main() {
    const fhead bmpfh = parse_fileheader(dummybmp, __crt_countof(dummybmp));
    assert(bmpfh.bfType == START_TAG_LE);
    assert(bmpfh.bfSize == 1409334); // size of the image where this buffer was extracted from, in bytes
    assert(bmpfh.bfReserved1 == 0);
    assert(bmpfh.bfReserved2 == 0);
    assert(bmpfh.bfOffBits == 54);

    const infhead bmpinfh = parse_infoheader(dummybmp, __crt_countof(dummybmp));
    assert(bmpinfh.biSize == 40); // header size
    assert(bmpinfh.biWidth == 734);
    assert(bmpinfh.biHeight == 480);
    assert(bmpinfh.biPlanes == 1);
    assert(bmpinfh.biBitCount == 32);
    assert(bmpinfh.biCompression == 0);
    assert(bmpinfh.biSizeImage == 0);
    assert(bmpinfh.biXPelsPerMeter == 3780);
    assert(bmpinfh.biYPelsPerMeter == 3780);
    assert(bmpinfh.biClrUsed == 0);
    assert(bmpinfh.biClrImportant == 0);

    const BITMAP_PIXEL_ORDERING order = get_pixel_order(&bmpinfh);
    assert(order == BOTTOMUP);

    // all of these test images will cause to_string to reroute to to_raw_string
    static const wchar_t* const filenames[] = { L"./test/bobmarley.bmp", L"./test/football.bmp",  L"./test/garfield.bmp",
                                                L"./test/gewn.bmp",      L"./test/girl.bmp",      L"./test/jennifer.bmp",
                                                L"./test/messi.bmp",     L"./test/supergirl.bmp", L"./test/time.bmp",
                                                L"./test/uefa2024.bmp",  L"./test/vendetta.bmp",  NULL };

    const wchar_t** _ptr                    = filenames;
    while (*_ptr) {
        bitmap_t             image = bmpread(*_ptr);
        const wchar_t* const wstr  = to_string(&image);
        if (!wstr) {
            wprintf_s(L"Error :: cannot process %s!\n", *_ptr);
            bmpclose(&image);
            continue;
        }

        _putws(wstr);
        _putws(L"\n\n");

        free(wstr);
        bmpclose(&image);
        _ptr++;
    }

    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
