#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

using namespace std;

class Solution {
public:
    int findMaxNumber(vector<int>& nums, int n) {
        string s = to_string(n);
        int len = s.size();
        sort(nums.begin(), nums.end());
        string best;

        string maxLess;
        int maxDigit = nums.back();
        for (int i = 0; i < len - 1; ++i) maxLess += (char)('0' + maxDigit);
        best = maxLess;

        // 再尝试构造 长度 == len 且 < n 的最大数
        function<bool(int, bool, string)> dfs = [&](int i, bool isLimit, string path) -> bool {
            if (i == len) {
                if (path < s) {
                    best = path;
                    return true;
                }
                return false;
            }
            // 从大到小枚举，贪心找最大
            for (auto it = nums.rbegin(); it != nums.rend(); ++it) {
                int d = *it;
                if (isLimit && d > (s[i] - '0')) continue;

                bool newLimit = isLimit && (d == s[i] - '0');
                if (dfs(i + 1, newLimit, path + char('0' + d))) {
                    return true;
                }
            }
            return false;
        };

        dfs(0, true, "");
        return stoi(best);
    }
};

int main() {
    vector<int> nums = {5,4,8,2};
    int n = 5416;
    Solution sol;
    cout << sol.findMaxNumber(nums, n) << endl; // 输出 5288
    return 0;
}
