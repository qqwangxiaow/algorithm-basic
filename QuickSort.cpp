#include <iostream>
#include <vector>



std::pair<int, int> partition3(std::vector<int>& nums, int l, int r) {
    int pivot = l + rand() % (r - l + 1);
    int p = nums[pivot];
    std::swap(nums[pivot], nums[r]);
    int lt = l, i = l, gt = r - 1;
    while(i <= gt) {
        if (nums[i] < p) {
            std::swap(nums[i++], nums[lt++]);
        } else if (nums[i] > p) {
            std::swap(nums[i], nums[gt--]);
        } else {
            i++;
        }
    }
    std::swap(nums[i], nums[r]);
    return {lt, i};
}

void quickSort(std::vector<int>& nums, int l, int r) {
    if (l >= r) {
        return;
    }
    auto p = partition3(nums, l, r);
    // +1， -1
    quickSort(nums, l, p.first - 1);
    quickSort(nums, p.second + 1, r); 
    return;
}

int main () {
    std::vector<int> vec = {1, 2, 3, 7, 9, 1, 4, 6, 9};
    quickSort(vec, 0, vec.size() - 1);
    for (auto each : vec) {
        std::cout << each << ", ";
    }
}