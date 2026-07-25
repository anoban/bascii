#include <array>
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
static const std::map<const char* const, long> TEST_BITMAPS {
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

static constexpr std::array<std::pair<int, int>, 11> TEST_BITMAPS_SHAPES {
    // in (w, h) format - in the same order as TEST_BITMAPS
    { { 140, 88 },
     { 106, 104 },
     { 120, 152 },
     { 135, 199 },
     { 140, 88 },
     { 138, 143 },
     { 120, 135 },
     { 130, 172 },
     { 112, 70 },
     { 100, 104 },
     { 140, 88 } }
};

[[maybe_unused]] static constexpr inline bool __attribute__((always_inline)) operator==(const fhead & left, const fhead & right) noexcept {
    return (left.type == right.type) && (left.size == right.size) && (left.offbits == right.offbits) && !left._reserved_0 &&
           !left._reserved_1 && !right._reserved_0 && !right._reserved_1;
}

TEST(bitmap, imopen) {
    long           fsize {};
    unsigned char* buffer {};
    for (const auto& pair : TEST_BITMAPS) {
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
    ASSERT_EQ(fsize, 0);  // the call above to imopen should've zeroed fsize
}

// lots of redundant file ios here :(

TEST(bitmap, fileheader) {
    long           fsize {};
    unsigned char* buffer {};
    fhead          header {};

    for (const auto& pair : TEST_BITMAPS) {
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
    unsigned       i {};
    long           fsize {};
    unsigned char* buffer {};
    infhead        header {};

    for (const auto& pair : TEST_BITMAPS) {
        buffer = ::imopen(pair.first, &fsize);
        header = ::infoheader(buffer, fsize);

        ASSERT_EQ(header.size, sizeof(infhead));
        ASSERT_EQ(header.width, TEST_BITMAPS_SHAPES[i].first);
        ASSERT_EQ(header.height, TEST_BITMAPS_SHAPES[i].second);
        ASSERT_EQ(header.planes, 1U);
        ASSERT_EQ(header.nbits, 32U);     // RGB - 32 bits
        ASSERT_FALSE(header.compression); // no compression in bitmaps
        ASSERT_FALSE(header.imagesize);
        // assert(header.ppm_x == 3780);
        // assert(header.ppm_y == 3780);
        ASSERT_FALSE(header.used_clrs); // must be 0
        ASSERT_FALSE(header.imp_clrs);  // must be 0

        ::free(buffer);
        buffer = nullptr;
        fsize  = 0;
        ::memset(&header, 0, sizeof(fhead));
        i++;
    }
}
