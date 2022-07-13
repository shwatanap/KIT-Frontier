#include <tdzdd/DdSpec.hpp>
#include <tdzdd/DdStructure.hpp>
#include <tdzdd/DdEval.hpp>

class KnapsackZdd : public tdzdd::DdSpec<KnapsackZdd, int, 2>
{
    int const n;
    int const *w;
    int const W;

public:
    KnapsackZdd(int n, int *w, int W) : n(n), w(w), W(W) {}

    int getRoot(int &state) const
    {
        state = 0;
        return (n);
    }

    int getChild(int &state, int level, int value)
        const
    {
        // printf("level: %d\n", level);
        if (value == 1)
            state += w[n - level];
        level--;
        if (state > W)
            return 0;
        if (level == 0)
            return -1;
        return level;
    }
};

class MaxElement : public tdzdd::DdEval<MaxElement, int>
{
    int const n;
    int const *c;

public:
    MaxElement(int n, int *c) : n(n), c(c) {}

    void evalTerminal(int &val, bool one) const
    {
        val = one ? 0 : INT_MIN;
    }

    void evalNode(int &val, int level, tdzdd::DdValues<int, 2> const &values) const
    {
        val = std::max(values.get(0), values.get(1) + c[n - level]);
    }
};

int main(int argc, char **argv)
{
    int n = 6, W = 15;
    int w[] = {4, 4, 3, 5, 8, 2};
    KnapsackZdd knapsack(n, w, W);
    tdzdd::DdStructure<2> dd(knapsack);
    // 1. 実行可能解の数
    std::cout << dd.zddCardinality() << std::endl;

    int c[] = {5, 8, 4, 6, 12, 4};
    // 2. 最適解
    std::cout << dd.evaluate(MaxElement(n, c)) << std::endl;

    // 3. 実行可能解の出力
    int i = 1;
    for (auto it = dd.begin(); it != dd.end(); ++it)
    {
        /* 実行可能解を１行ずつ出力 */
        std::cout << i << ": ";
        for (auto itr = (*it).begin(); itr != (*it).end(); ++itr)
        {
            std::cout << n - *itr + 1 << " ";
        }
        i++;
        std::cout << std::endl;
    }
    // 0, 1で出力
    // for (auto &knapsack_set : dd)
    // {
    //     for (int i = 0; i < n; ++i)
    //     {
    //         int level = 6 - i;
    //         std::cout << knapsack_set.count(level);
    //     }
    //     std::cout << std::endl;
    // }
}
