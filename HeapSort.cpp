#include<iostream>
#include<vector>
void adjust(std::vector<int>& nums, int i, int size) {
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    int max_index = i;
    if (left < size && nums[max_index] < nums[left]) {
        max_index = left;
    }
    if (right < size && nums[max_index] < nums[right]) {
        max_index = right;
    }
    if (max_index != i) {
        std::swap(nums[i], nums[max_index]);
        adjust(nums, max_index, size);
    }
    return;

}

void heapSort(std::vector<int>& nums) {
    int n = nums.size();
    // 要-1
    for (int i = n / 2 - 1; i >= 0; --i) {
        adjust(nums, i, n);
    }
    for (int i = n - 1 ; i > 0; --i) {
        std::swap(nums[i], nums[0]);
        // 从 0 开始 size 是 i
        adjust(nums, 0, i);
    }
}

int main()
{
    std::vector<int> arr{2,1,9,-1,6,0,22};
    heapSort(arr);
    for(int i = 0; i < arr.size(); i++) {
        std::cout << " " << arr[i];
    }

}


