#include "check.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

struct Interval { int start, end; };
// 半开区间 [start,end)，要求 start < end；端点接触可以兼容。
int schedule(std::vector<Interval> intervals) {
    std::sort(intervals.begin(), intervals.end(), [](auto a, auto b) { return a.end < b.end; });
    int count = 0, end = std::numeric_limits<int>::min();
    for (auto interval : intervals) if (interval.start >= end) {
        ++count; end = interval.end;
    }
    return count;
}
// 小规模穷举：检查任意两段是否重叠，独立于贪心排序策略。
int brute_schedule(const std::vector<Interval>& intervals) {
    int best = 0;
    for (unsigned mask = 0; mask < (1u << intervals.size()); ++mask) {
        bool valid = true; int count = 0;
        for (std::size_t i = 0; i < intervals.size(); ++i) if (mask & (1u << i)) {
            ++count;
            for (std::size_t j = 0; j < i; ++j) if (mask & (1u << j))
                if (std::max(intervals[i].start, intervals[j].start) <
                    std::min(intervals[i].end, intervals[j].end)) valid = false;
        }
        if (valid) best = std::max(best, count);
    }
    return best;
}
int main() {
    std::vector<Interval> candidates{{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}};
    for (unsigned mask = 0; mask < 64; ++mask) {
        std::vector<Interval> a;
        for (unsigned i = 0; i < 6; ++i) if (mask & (1u << i)) a.push_back(candidates[i]);
        check(schedule(a) == brute_schedule(a), "greedy vs subsets");
    }
    check(schedule({{-3, -1}, {-1, 0}, {-2, 2}}) == 2, "negative endpoints");
    std::cout << "maximum compatible intervals = " << schedule({{0, 3}, {0, 1}, {1, 2}, {2, 3}}) << '\n';
}
