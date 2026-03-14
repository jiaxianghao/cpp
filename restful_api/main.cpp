#include <iostream>
#include <string>
#include <vector>

using namespace std;
class Solution {
public:
    int expand(const string& s, int left, int right)
    {
        while(left >= 0 && right < s.size() && s[left] == s[right])
        {
            left--;
            right++;
        }
        return (right - left - 2) / 2;
    }
    string longestPalindrome(string s) {
        int j = -1;
        int right = -1;
        int resCenter = 0;
        int resArm = 0;

        string temp_str = "#";
        for(auto e : s)
        {
            temp_str = temp_str + e + "#";
        }
        s = temp_str;

        int n = s.size();
        vector<int> arm_len;
        for(int i = 0; i < n; i++)
        {
            int arm = 0;
            if(i <= right)
            {
                int min_arm = min(arm_len[2 * j - i], right - i);
                arm = expand(s, i - min_arm, i + min_arm);
            }
            else
            {
                arm = expand(s, i, i);
            }
            
            arm_len.push_back(arm);

            if(arm > resArm)
            {
                resCenter = i;
                resArm = arm;
            }
            if(i + arm > right)
            {
                j = i;
                right = i + arm;
            }
        }
        
        string res;
        for(int i = resCenter -  resArm; i <= resCenter + resArm; ++i)
        {
            if(s[i] != '#')
            {
                res += s[i];
            }
        }
        return res;
    }

};


int main(int, char**){
    #if MAXXXX
    std::cout << "hello" << std::endl;
    #endif
    std::cout << "world" << std::endl;

    Solution s;
    auto res = s.longestPalindrome("babad");
    return 0;
}
