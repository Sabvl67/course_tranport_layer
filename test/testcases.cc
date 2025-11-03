#include <gtest/gtest.h>
#include "header_driver_t.h"

class ExtraHeadersTest : public ::testing::Test {
protected:
    static const int SIZE = 12;
    unsigned char buffer[SIZE];

    void SetUp() override {
        for (int i = 0; i < SIZE; ++i) buffer[i] = 0xAA; // pattern fill
    }
};

// Tests for Type field
TEST_F(ExtraHeadersTest, TypeBoundaryValues) {
    setType_t(buffer, SIZE, 0);
    EXPECT_EQ(getType_t(buffer, SIZE), 0u);

    setType_t(buffer, SIZE, 3);
    EXPECT_EQ(getType_t(buffer, SIZE), 3u);
}

TEST_F(ExtraHeadersTest, TypeOutOfRangeMasked) {
    setType_t(buffer, SIZE, 7); // 111b
    EXPECT_EQ(getType_t(buffer, SIZE), 3u); // only 2 bits kept
}

// Tests for Window field
TEST_F(ExtraHeadersTest, WindowBoundaryValues) {
    setWin_t(buffer, SIZE, 0);
    EXPECT_EQ(getWin_t(buffer, SIZE), 0u);

    setWin_t(buffer, SIZE, 31);
    EXPECT_EQ(getWin_t(buffer, SIZE), 31u);
}

TEST_F(ExtraHeadersTest, WindowOutOfRangeMasked) {
    setWin_t(buffer, SIZE, 255);
    EXPECT_EQ(getWin_t(buffer, SIZE), 31u);
}

// ---- Seq num ----
TEST_F(ExtraHeadersTest, SeqBoundaryValues) {
    setSeq_t(buffer, SIZE, 0);
    EXPECT_EQ(getSeq_t(buffer, SIZE), 0u);

    setSeq_t(buffer, SIZE, 255);
    EXPECT_EQ(getSeq_t(buffer, SIZE), 255u);
}

TEST_F(ExtraHeadersTest, SeqOutOfRangeMasked) {
    setSeq_t(buffer, SIZE, 1024); // 0x400
    EXPECT_EQ(getSeq_t(buffer, SIZE), 0u); // only low 8 bits kept
}

// Tests for Length
TEST_F(ExtraHeadersTest, LengthBoundaryValues) {
    setLen_t(buffer, SIZE, 0);
    EXPECT_EQ(getLen_t(buffer, SIZE), 0u);

    setLen_t(buffer, SIZE, 65535);
    EXPECT_EQ(getLen_t(buffer, SIZE), 65535u);
}

TEST_F(ExtraHeadersTest, LengthOutOfRangeMasked) {
    setLen_t(buffer, SIZE, 70000); // > 16 bits
    EXPECT_EQ(getLen_t(buffer, SIZE), 4464u); // 70000 mod 65536
}

// Tests for CRC1
TEST_F(ExtraHeadersTest, CRC1BoundaryValues) {
    setCRC1_t(buffer, SIZE, 0);
    EXPECT_EQ(getCRC1_t(buffer, SIZE), 0u);

    setCRC1_t(buffer, SIZE, 0xFFFFFFFF);
    EXPECT_EQ(getCRC1_t(buffer, SIZE), 0xFFFFFFFFu);
}

TEST_F(ExtraHeadersTest, CRC1OutOfRangeMasked) {
    setCRC1_t(buffer, SIZE, 0x1FFFFFFFF); // 33 bits
    EXPECT_EQ(getCRC1_t(buffer, SIZE), 0xFFFFFFFFu); // truncated
}

// Tests for Buffer integrity
TEST_F(ExtraHeadersTest, OnlyTouchesExpectedBytes) {
    // Save original pattern
    unsigned char before[SIZE];
    for (int i = 0; i < SIZE; ++i) before[i] = buffer[i];

    setSeq_t(buffer, SIZE, 123);

    // Only buffer[1] should differ
    for (int i = 0; i < SIZE; ++i) {
        if (i == 1) {
            EXPECT_EQ(buffer[i], 123);
        } else {
            EXPECT_EQ(buffer[i], before[i]);
        }
    }
}

// Tests for Small buffer safety
TEST(ExtraHeadersSafety, NullBufferDoesNotCrash) {
    setType_t(nullptr, 0, 2);
    EXPECT_EQ(getType_t(nullptr, 0), 0u);
}

TEST(ExtraHeadersSafety, TooSmallBuffer) {
    unsigned char tiny[2] = {0};
    setCRC1_t(tiny, 2, 0xDEADBEEF); // should no-op
    EXPECT_EQ(getCRC1_t(tiny, 2), 0u);
}

