#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

using namespace std;

int findMaxNumber(vector<int>& nums, int n) {

    string s = to_string(n);
    std::sort(nums.begin(), nums.end());

    string maxLess;
    int len = s.size();
    for (int i = 0; i < len -1; ++i) {
        maxLess += char(nums.back() + '0');
    }
    string best = maxLess;
    std::function<bool(int, bool, string)> dfs = [&] (int i, bool isLimit, string path) {
        if (i == len) {
            if (path < s) {
                best = path;
                return true;
            }
            return false;
        }
        
        for (auto it = nums.rbegin(); it != nums.rend(); it++) {
            int d = *it;
            if (isLimit && d > s[i] - '0') {
                continue;
            }
            bool newLimit = isLimit && (d == s[i] - '0');
            if(dfs(i + 1, newLimit, path + char(d + '0'))) {
                return true;
            }

        }
        return false;
    };

    dfs(0, false, "");
    return stoi(best);

};

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

