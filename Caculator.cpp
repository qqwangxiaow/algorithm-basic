#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <algorithm>
#include <numeric>

int calculate(std::string s) {
    std::function<long long(int&)> dfs = [&] (int& i) {
        // long long
        std::vector<long long> stk;
        long long num = 0;
        char pre_sign = '+';
        for ( ; i < s.size(); ++i) {
            if (isdigit(s[i])) {
                num = num * 10 + s[i] - '0';
            } else if (s[i] == '(') {
                ++i;
                num = dfs(i);
            }
            // == size -1
            if ((!isdigit(s[i]) && s[i] != ' ') || (i == s.size() - 1)) {
                switch(pre_sign) {
                    case '+': stk.push_back(num); break;
                    case '-': stk.push_back(-num); break;
                    case '*': stk.back() *= num;break;
                    case '/': stk.back() /= num;break;
                    default: break;
                }
                if (s[i] == ')') {
                    //++
                    i++;
                    return accumulate(stk.begin(), stk.end(), 0LL);
                }
                num = 0;
                // s[i]
                pre_sign = s[i];
            }
        }
        return accumulate(stk.begin(), stk.end(), 0LL);
    };
    int temp = 0;
    return dfs(temp);
};
int main() {
    std::string test = "1 - 1 + 1 + 2 / 1 - (1 - 2)";
    std::cout << "result: " << calculate(test) << std::endl;
    
}