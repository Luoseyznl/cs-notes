# 刷题思想

提高刷题的应试能力是我的核心目标，主要分为两步：

1. 跟着灵茶的专题路线提高专项能力，每天 1 题左右，重点是建立“题目与解题方法之间的关系”（题感）。（每天一小时）
2. 每日一题 + 每周打周赛，先去尝试提高应试能力，做的咋样无所谓。（每周日上午 10:30~12:00）

---

# 刷题日记

## 2026.5.24

1. [1456. 定长子串中元音的最大数目] 4

给你字符串 s 和整数 k 。请返回字符串 s 中长度为 k 的单个子字符串中可能包含的最大元音字母数。英文中的元音字母为（a, e, i, o, u）。

```cpp
class Solution {
    bool isVowel(char c) {
        return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
    }

public:
    int maxVowels(string s, int k) {
        int n = s.length();
        int curr_vowels = 0;

        for (int i = 0; i < k; ++i) {
            if (isVowel(s[i])) {
                curr_vowels++;
            }
        }

        int max_vowels = curr_vowels;

        for (int i = k; i < n; ++i) {
            if (isVowel(s[i])) {
                curr_vowels++;
            }
            if (isVowel(s[i - k])) {
                curr_vowels--;
            }
            max_vowels = max(max_vowels, curr_vowels);
        }
        return max_vowels;
    }
};
```

> **定长滑动窗口题**，可以分成两块 for 完成，注意在第一个 for 之后要存一下结果。

2. [643. 子数组最大平均数 I] 3

给你一个由 n 个元素组成的整数数组 nums 和一个整数 k。请你找出平均数最大且长度为 k 的连续子数组，并输出该最大平均数。任何误差小于 10-5 的答案都将被视为正确答案。

```cpp
class Solution {

public:
    double findMaxAverage(vector<int>& nums, int k) {
        int n = nums.size();
        double curr_sum = 0;

        for (int i = 0; i < k; ++i) {
            curr_sum += nums[i];
        }

        double max_sum = curr_sum;

        for (int i = k; i < n; ++i) {
            curr_sum += nums[i];
            curr_sum -= nums[i - k];
            max_sum = max(max_sum, curr_sum);
        }

        return max_sum / k;
    }
};
```

> 和上题一样。性能压榨：在循环体内尽量不做 `/`，留到最后 `return max_sum / k;` 就行。
