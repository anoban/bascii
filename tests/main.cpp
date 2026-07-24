#include <array>
#include <ctime>
#include <map>

#ifndef __VERBOSE_OUTPUTS
    #define __VERBOSE_OUTPUTS 1
#endif

#ifndef __TEST__
    #define __TEST__ 1
#endif

// clang-format off
#include <gtest/gtest.h>
#include <tostring.h>
// clang-format on

// all of these test images will cause to_string to reroute to to_raw_string
static const std::map<const char*, unsigned> bitmaps {
    { "./images/tests/bobmarley.bmp",  49334 },
    {  "./images/tests/football.bmp",  44150 },
    {  "./images/tests/garfield.bmp",  73014 },
    {      "./images/tests/gewn.bmp", 107514 },
    {      "./images/tests/girl.bmp",  49334 },
    {  "./images/tests/jennifer.bmp",  78990 },
    {     "./images/tests/messi.bmp",  64854 },
    { "./images/tests/supergirl.bmp",  89494 },
    {      "./images/tests/time.bmp",  31414 },
    {  "./images/tests/uefa2024.bmp",  41654 },
    {  "./images/tests/vendetta.bmp",  49334 }
};

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

//-------------
// <_bitmap.h>
//-------------

[[maybe_unused]] static constexpr inline bool __attribute__((always_inline)) operator==(const fhead & left, const fhead & right) noexcept {
    return (left.type == right.type) && (left.size == right.size) && (left.offbits == right.offbits) && !left._reserved_0 &&
           !left._reserved_1 && !right._reserved_0 && !right._reserved_1;
}

TEST(bitmap, imopen) {
    //
    long           fsize {};
    unsigned char* buffer {};
    for (const auto& pair : bitmaps) {
        buffer = ::imopen(pair.first, &fsize);
        ASSERT_TRUE(buffer);           // cannot be nullptr
        ASSERT_EQ(fsize, pair.second); // file sizes should match
        ::free(buffer);
        buffer = nullptr;
        fsize  = 0;
    }

    fsize  = 746; // for testing
    buffer = ::imopen("./a/file/that/does/not/exist.bmp", &fsize);
    ASSERT_FALSE(buffer); // nullptr
    ASSERT_EQ(fsize, 0);
}

// lots of redundant file ios here :/

TEST(bitmap, fileheader) {
    //
    long           fsize {};
    unsigned char* buffer {};
    fhead          header {};

    for (const auto& pair : bitmaps) {
        buffer = ::imopen(pair.first, &fsize);

        header = ::fileheader(buffer, fsize);

        ASSERT_EQ(header.type, START_TAG_LE);
        ASSERT_EQ(header.size, pair.second);
        ASSERT_EQ(header._reserved_0, 0);
        ASSERT_EQ(header._reserved_1, 0);
        ASSERT_EQ(header.offbits, 54);

        ::free(buffer);
        buffer = nullptr;
        fsize  = 0;
        ::memset(&header, 0, sizeof(fhead));
    }

    // test for failure cases
    ASSERT_EQ(::fileheader(nullptr, 200), fhead {}); // sould fail because of the null buffer
                                                     // shoudl fail because the buffer size is smaller than the size of the file header too
    ASSERT_EQ(::fileheader(reinterpret_cast<unsigned char*>(0x75A45D23), 2), fhead {});
    ASSERT_EQ(::fileheader(nullptr, 2), fhead {}); // fail for both reasons above
}

TEST(bitmap, infoheader) {
    //
    long           fsize {};
    unsigned char* buffer {};
    infhead        header {};

    for (const auto& pair : bitmaps) {
        buffer = ::imopen(pair.first, &fsize);

        header = ::infoheader(buffer, fsize);

        // ASSERT_EQ(header.type, START_TAG_LE);
        // ASSERT_EQ(header.size, pair.second);
        // ASSERT_EQ(header._reserved_0, 0);
        // ASSERT_EQ(header._reserved_1, 0);
        // ASSERT_EQ(header.offbits, 54);

        ::free(buffer);
        buffer = nullptr;
        fsize  = 0;
        ::memset(&header, 0, sizeof(fhead));
    }
}

// make sure that all the basic mappers don't compute off-bound offsets for palettes
TEST(basic_mappers, arithmetic) {
    rgbq test { 0x00, 0x00, 0x00, 0xFF };

    for (unsigned b = 0; b <= UCHAR_MAX; ++b) {
        for (unsigned g = 0; g <= UCHAR_MAX; ++g) {
            for (unsigned r = 0; r <= UCHAR_MAX; ++r) {
                test.b = b;
                test.g = g;
                test.r = r;

                ASSERT_NE(::memchr(PALETTE_BASE, ::arithmetic(&test, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)), nullptr);
                ASSERT_NE(
                    ::memchr(PALETTE_MINIMAL, ::arithmetic(&test, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)), sizeof(PALETTE_MINIMAL)),
                    nullptr
                );
                ASSERT_NE(
                    ::memchr(PALETTE_EXTENDED, ::arithmetic(&test, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)), sizeof(PALETTE_EXTENDED)),
                    nullptr
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

                ASSERT_NE(::memchr(PALETTE_BASE, ::weighted(&test, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)), nullptr);
                ASSERT_NE(
                    ::memchr(PALETTE_MINIMAL, ::weighted(&test, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)), sizeof(PALETTE_MINIMAL)), nullptr
                );
                ASSERT_NE(
                    ::memchr(PALETTE_EXTENDED, ::weighted(&test, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)), sizeof(PALETTE_EXTENDED)),
                    nullptr
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

                ASSERT_NE(::memchr(PALETTE_BASE, ::minmax(&test, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)), nullptr);
                ASSERT_NE(
                    ::memchr(PALETTE_MINIMAL, ::minmax(&test, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)), sizeof(PALETTE_MINIMAL)), nullptr
                );
                ASSERT_NE(
                    ::memchr(PALETTE_EXTENDED, ::minmax(&test, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)), sizeof(PALETTE_EXTENDED)),
                    nullptr
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

                ASSERT_NE(::memchr(PALETTE_BASE, ::luminosity(&test, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)), nullptr);
                ASSERT_NE(
                    ::memchr(PALETTE_MINIMAL, ::luminosity(&test, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)), sizeof(PALETTE_MINIMAL)),
                    nullptr
                );
                ASSERT_NE(
                    ::memchr(PALETTE_EXTENDED, ::luminosity(&test, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)), sizeof(PALETTE_EXTENDED)),
                    nullptr
                );
            }
        }
    }
}

int main() {
    /*
    const fhead bmpfh = parse_fileheader(dummybmp, __crt_countof(dummybmp));


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
    */

    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
