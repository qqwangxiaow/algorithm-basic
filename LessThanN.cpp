#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

using namespace std;

int lessN(std::vector<int>& nums, int n) {
    std::sort(nums.begin(), nums.end());
    std::string s = std::to_string(n);
    // s.size
    int m = s.size();
    std::string maxbest;
    for (int i = 0; i < m - 1; ++i) {
        maxbest += char(nums.back() + '0');
    }
    std::function<bool(int, bool, std::string)> dfs = [&] (int i, bool is_limit, std::string path) {
        if (i == m) {
            // 这里是 s
            if (path < s) {
                maxbest = path;
                return true;
            }
            return false;
        }
        for (auto iter = nums.rbegin(); iter != nums.rend(); ++iter) {
            int d = *iter;
            // is_limit
            if (is_limit && (d > s[i] - '0')) {
                continue;
            }
            // s和 nums 的关系别写错了
            int up = is_limit ? s[i] - '0' : 9;
            // 这里不用循环遍历 贪心就够了
            if (dfs(i + 1, is_limit && d == up, path + char(d + '0'))) {
                return true;
            }
        };
        return false;
    };
    dfs(0, true, "");
    if (maxbest.empty()) {
        return -1;
    }
    return std::stoi(maxbest);
}

int atMostNGivenDigitSet(vector<string>& digits, int n) {
    std::string s = to_string(n);
    int m = s.size();
    vector<int> dp(m + 1, -1);
    std::function<int(int, bool, bool)> dfs = [&] (int i, bool isLimit, bool isNum) {
        if (i == m) {
            return isNum;
        }
        if (!isLimit && isNum && dp[i] >= 0) {
            return dp[i];
        }
        int ret = 0;
        if (!isNum) {
            ret += dfs(i + 1, false, false);
        }
        char up = isLimit ? s[i] : '9';
        for (int j = 0; j < digits.size(); ++j) {
            char d = digits[j][0];
            if (d > up) {
                break;
            }
            ret += dfs(i + 1, isLimit && d == up, true);
        }
        if (!isLimit && isNum) {
            dp[i] = ret;
        }
        return ret;
    };
    return dfs(0, true, false);
}


int main() {
    vector<int> nums = {5,4,8,2};
    int n = 5416;
    Solution sol;
    cout << findMaxNumber(nums, n) << endl; // 输出 5288
    return 0;
}

