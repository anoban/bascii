#include <utils.h>

// clang-format off
#include <gtest/gtest.h>
#include <tostring.h>
// clang-format on

// --------------------------------------------------------------------------------------
// make sure that all the basic mappers don't compute off-bound offsets for palettes
// --------------------------------------------------------------------------------------

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

// --------------------------------------------------------------------------------------
// make sure that all the block mappers don't compute off-bound offsets for palettes
// --------------------------------------------------------------------------------------

TEST(block_mappers, arithmetic) {
    for (unsigned b = 0; b <= UCHAR_MAX; ++b) {
        for (unsigned g = 0; g <= UCHAR_MAX; ++g) {
            for (unsigned r = 0; r <= UCHAR_MAX; ++r) {
                ASSERT_NE(
                    ::memchr(PALETTE_BASE, ::arithmetic_blockmapper(b, g, r, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)),
                    nullptr
                );
                ASSERT_NE(
                    ::memchr(
                        PALETTE_MINIMAL,
                        ::arithmetic_blockmapper(b, g, r, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)),
                        sizeof(PALETTE_MINIMAL)
                    ),
                    nullptr
                );
                ASSERT_NE(
                    ::memchr(
                        PALETTE_EXTENDED,
                        ::arithmetic_blockmapper(b, g, r, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)),
                        sizeof(PALETTE_EXTENDED)
                    ),
                    nullptr
                );
            }
        }
    }
}

TEST(block_mappers, weighted) {
    for (unsigned b = 0; b <= UCHAR_MAX; ++b) {
        for (unsigned g = 0; g <= UCHAR_MAX; ++g) {
            for (unsigned r = 0; r <= UCHAR_MAX; ++r) {
                ASSERT_NE(
                    ::memchr(PALETTE_BASE, ::weighted_blockmapper(b, g, r, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)),
                    nullptr
                );
                ASSERT_NE(
                    ::memchr(
                        PALETTE_MINIMAL, ::weighted_blockmapper(b, g, r, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)), sizeof(PALETTE_MINIMAL)
                    ),
                    nullptr
                );
                ASSERT_NE(
                    ::memchr(
                        PALETTE_EXTENDED,
                        ::weighted_blockmapper(b, g, r, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)),
                        sizeof(PALETTE_EXTENDED)
                    ),
                    nullptr
                );
            }
        }
    }
}

TEST(block_mappers, minmax) {
    for (unsigned b = 0; b <= UCHAR_MAX; ++b) {
        for (unsigned g = 0; g <= UCHAR_MAX; ++g) {
            for (unsigned r = 0; r <= UCHAR_MAX; ++r) {
                ASSERT_NE(
                    ::memchr(PALETTE_BASE, ::minmax_blockmapper(b, g, r, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)), nullptr
                );
                ASSERT_NE(
                    ::memchr(
                        PALETTE_MINIMAL, ::minmax_blockmapper(b, g, r, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)), sizeof(PALETTE_MINIMAL)
                    ),
                    nullptr
                );
                ASSERT_NE(
                    ::memchr(
                        PALETTE_EXTENDED,
                        ::minmax_blockmapper(b, g, r, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)),
                        sizeof(PALETTE_EXTENDED)
                    ),
                    nullptr
                );
            }
        }
    }
}

TEST(block_mappers, luminosity) {
    for (unsigned b = 0; b <= UCHAR_MAX; ++b) {
        for (unsigned g = 0; g <= UCHAR_MAX; ++g) {
            for (unsigned r = 0; r <= UCHAR_MAX; ++r) {
                ASSERT_NE(
                    ::memchr(PALETTE_BASE, ::luminosity_blockmapper(b, g, r, PALETTE_BASE, sizeof(PALETTE_BASE)), sizeof(PALETTE_BASE)),
                    nullptr
                );
                ASSERT_NE(
                    ::memchr(
                        PALETTE_MINIMAL,
                        ::luminosity_blockmapper(b, g, r, PALETTE_MINIMAL, sizeof(PALETTE_MINIMAL)),
                        sizeof(PALETTE_MINIMAL)
                    ),
                    nullptr
                );
                ASSERT_NE(
                    ::memchr(
                        PALETTE_EXTENDED,
                        ::luminosity_blockmapper(b, g, r, PALETTE_EXTENDED, sizeof(PALETTE_EXTENDED)),
                        sizeof(PALETTE_EXTENDED)
                    ),
                    nullptr
                );
            }
        }
    }
}
