#include <iostream>
#include <vector>

void mergeSort(std::vector<int>& nums, std::vector<int>& temp, int l, int r) {
    if (l >= r) {
        return;
    }
    int m = l + (r - l) / 2;
    mergeSort(nums, temp, l , m);
    mergeSort(nums, temp, m + 1, r);
    // 优化, 避免拷贝
    if (nums[m] <= nums[m + 1]) {
        return;
    }
    int l_index = l;
    int r_index = m + 1;
    int i = l;
    while(l_index <= m && r_index <= r) {
        temp[i++] = nums[l_index] <= nums[r_index] ? nums[l_index++] : nums[r_index++];
    }
    while(l_index <= m) {
        temp[i++] = nums[l_index++];
    }
    while(r_index <= r) {
        temp[i++] = nums[r_index++];
    }
    
    // need copy
    for (i = l; i <=r; ++i) {
        nums[i] = temp[i];
    }
}

void sort(std::vector<int>& nums) {
    int n = nums.size();
    if (n <= 1) {
        return;
    }
    std::vector<int> temp(n);
    mergeSort(nums, temp, 0, n - 1);
}

int main() {
    std::vector<int> temp {-1, 3, 1, 2, 5, 3};
    sort(temp);
    for (auto each : temp) {
        std::cout << "each: " << each << std::endl;
    }
}