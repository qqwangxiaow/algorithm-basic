#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

int longestCommonSubsequence(std::string text1, std::string text2) {
    int m = text1.size(), n = text2.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (text1[i] == text2[j]) {
                dp[i + 1][j + 1] = dp[i][j] + 1; 
            } else {
                dp[i + 1][j + 1] = std::max(dp[i][j + 1], dp[i + 1][j]);
            }
        }
    }
    return dp[m][n];

}

std::string getLongestCommonSubsequence(const std::string& text1, const std::string& text2) {
    int m = text1.size(), n = text2.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (text1[i] == text2[j]) {
                dp[i + 1][j + 1] = dp[i][j] + 1;
            } else {
                dp[i + 1][j + 1] = std::max(dp[i][j + 1], dp[i + 1][j]);
            }
        }
    }
    /*
    回溯规则很简单：
        如果 text1[i - 1] == text2[j - 1]，说明这个字符属于 LCS，把它加入答案，然后 i--, j--
        如果不相等，就看 dp[i - 1][j] 和 dp[i][j - 1]
        谁大就往谁那个方向走，因为那个方向保留了更长的公共子序列
    */
    std::string lcs;
    int i = m;
    int j = n;
    while (i > 0 && j > 0) {
        if (text1[i - 1] == text2[j - 1]) {
            lcs.push_back(text1[i - 1]);
            --i;
            --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            --i;
        } else {
            --j;
        }
    }

    std::reverse(lcs.begin(), lcs.end());
    return lcs;

}

int main() {
    std::string text1 = "abcde";
    std::string text2 = "ace";

    std::cout << "length: " << longestCommonSubsequence(text1, text2) << std::endl;
    std::cout << "lcs: " << getLongestCommonSubsequence(text1, text2) << std::endl;
}