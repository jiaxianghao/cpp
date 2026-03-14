#include <gtest/gtest.h>
#include "../mock_lib/uos_rmap_mock.h"

using ::testing::_;
using ::testing::Return;

TEST(OpRmapParseMockTest, LoadRmapUserZoneSuccess) {
    opRmapParseMock mockRmapParse;

    // 设置期望行为：load_rmap_user_zone 被调用一次，并返回 0（成功）
    EXPECT_CALL(mockRmapParse, load_rmap_user_zone())
        .Times(1)
        .WillOnce(Return(0));

    // 调用 Mock 方法
    int result = mockRmapParse.load_rmap_user_zone();

    // 验证结果
    ASSERT_EQ(result, 0);
}

TEST(OpRmapParseMockTest, LoadRmapUserZoneFailure) {
    opRmapParseMock mockRmapParse;

    // 设置期望行为：load_rmap_user_zone 被调用一次，并返回 -1（失败）
    EXPECT_CALL(mockRmapParse, load_rmap_user_zone())
        .Times(1)
        .WillOnce(Return(-1));

    // 调用 Mock 方法
    int result = mockRmapParse.load_rmap_user_zone();

    // 验证结果
    ASSERT_EQ(result, -1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}