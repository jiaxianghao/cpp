#ifndef __UOS_RMAP_MOCK_H__
#define __UOS_RMAP_MOCK_H__

#include <gmock/gmock.h>
#include <string>

// 使用 int 替代 return_t
class IopRmapParse {
public:
    virtual ~IopRmapParse() {}

    virtual int load_rmap_user_zone() = 0;
};

class opRmapParseMock : public IopRmapParse {
public:
    MOCK_METHOD0(load_rmap_user_zone, int());
};

#endif /* __UOS_RMAP_MOCK_H__ */