int binary_search(vector<int>& nums, int target) {
    int lo = 0, hi = nums.size() - 1; // 在闭区间 [lo, hi] 中查找
    // 退出查找的条件是：查找范围为空
    while (lo <= hi) {
        int mi = lo + (hi - lo) / 2;
        if (nums[mi] < target) {
            lo = mi + 1; // mi 不是可行解，范围变成 [mi+1, hi]
        } else if (nums[mi] > target) {
            hi = mi - 1; // mi 不是可行解，范围变成 [lo, mi-1]
        } else {
            return mi;
        }
    }
    return -1;
}
//存在重复元素 找第一个
int binary_search(vector<int>& nums,int target) {
    int lo = 0, hi = nums.size() - 1; // 在闭区间 [lo, hi] 中查找
    // 退出查找的条件是：查找范围为1个元素
    while (lo < hi) {
        int mi = lo + (hi - lo) / 2;
        if (nums[mi] == target) {
            hi = mi; // mi 在可行解区间中，可行解区间范围缩小为 [lo, mi]
        } else {
            lo = mi + 1; // mi 不在可行解区间中，可行解区间范围缩小为 [mi+1, hi]
        }
    }
    return lo;
}
//存在重复元素 找最后一个
int binary_search(vector<int>& nums,int target) {
    int lo = 0, hi = nums.size() - 1; // 在闭区间 [lo, hi] 中查找
    // 退出查找的条件是：查找范围为1个元素
    while (lo < hi) {
        int mi = lo + (hi - lo + 1) / 2; // +1 保证可行解区间范围缩小
        if (nums[mi] == target) {
            lo = mi; // mi 在可行解区间中，可行解区间范围缩小为 [mi, hi]
        } else {
            hi = mi - 1; // mi 不在可行解区间中，可行解区间范围缩小为 [lo, mi-1]
        }
    }
    return lo;
}