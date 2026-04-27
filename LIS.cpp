#include <iostream>
#include <vector>
#include <cstddef>
#include <algorithm>

int longIncreaseSequence(std::vector<int>& nums, std::vector<int>* seq) {
    int n = nums.size();
    std::vector<int> dp(n, 1);
    std::vector<int> prev(n, -1);
    for (int i = 1; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            if (nums[j] < nums[i]) {
                if (dp[i] < dp[j] + 1) {
                    prev[i] = j; 
                    dp[i] = dp[j] + 1;
                }
            }
        }
    }
    int index = max_element(dp.begin(), dp.end()) - dp.begin();
    //i != -1, not prev i
    for (int i = index; i != -1; i = prev[i]) {
        seq->push_back(nums[i]);
    }
    reverse(seq->begin(), seq->end());
    return *max_element(dp.begin(), dp.end());
}

int main () {
    std::vector<int> nums = {1, 2, 7, 8, 9, 6, 7, 8, 9, 2};
    std::vector<int> seq;
    std::cout << longIncreaseSequence(nums, &seq) << std::endl;
    for (int i = 0; i < seq.size(); ++i) {
        std::cout << seq[i];
    }
}