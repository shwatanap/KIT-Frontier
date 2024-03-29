#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <tdzdd/DdSpec.hpp>
#include <tdzdd/DdStructure.hpp>
#include <tdzdd/DdEval.hpp>
#include <tdzdd/DdSpecOp.hpp>

#define PRUNING 0
#define SUCCESS -1

// 集合族を表すクラス FamilyofSets
class FamilyofSets
{
private:
	int const n;
	int const m;
	std::vector<int> const set_weight;
	std::vector<std::vector<int>> const set_list;

public:
	FamilyofSets(int n, int m, std::vector<int> set_weight, std::vector<std::vector<int>> set_list)
		: n(n), m(m), set_weight(set_weight), set_list(set_list)
	{
	}

	int getWeight(int id) const
	{
		if (id < 1 || id > m)
		{
			std::cerr << "Error: " << id << "is not element ID." << std::endl;
			exit(-1);
		}
		return set_weight[id - 1];
	}

	std::vector<int> getSet(int id) const
	{
		if (id < 1 || id > m)
		{
			std::cerr << "Error: " << id << "is not element ID." << std::endl;
			exit(-1);
		}
		return set_list[id - 1];
	}

	int numElements() const
	{
		return n;
	}

	int numSets() const
	{
		return m;
	}

	void print() const
	{
		std::cout << "----- Output -------" << std::endl;
		std::cout << "#Elements: " << numElements() << std::endl;
		std::cout << "#Sets: " << numSets() << std::endl;
		for (int i = 0; i < set_list.size(); i++)
		{
			std::cout << getWeight(i + 1) << ": ";
			for (auto it = set_list[i].begin(); it != set_list[i].end(); ++it)
			{
				std::cout << *it << "  ";
			}
			std::cout << std::endl;
		}
		std::cout << "--------------------------" << std::endl;
	}
};

// 入力ファイルの読み込み
FamilyofSets readSets(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cerr << "Please input a data file" << std::endl;
		exit(-1);
	}
	std::ifstream ifs(argv[1]);
	if (!ifs)
	{
		std::cerr << "Error: cannot open the file " << argv[1] << std::endl;
		exit(-1);
	}

	int max = 0;
	std::vector<int> weight;
	std::vector<std::vector<int>> set_list;
	std::string line;
	while (getline(ifs, line))
	{
		std::vector<int> v;
		std::stringstream ss(line);
		bool flag = true;
		int element;
		while (ss >> element)
		{
			if (flag)
			{
				weight.push_back(element);
				flag = false;
			}
			else
			{
				v.push_back(element);
				if (element > max)
					max = element;
			}
		}
		set_list.push_back(v);
	}

	FamilyofSets F(max, set_list.size(), weight, set_list);
	return F;
}

class SetZDD : public tdzdd::PodArrayDdSpec<SetZDD, int, 2>
{
	FamilyofSets const F;

public:
	SetZDD(FamilyofSets F) : F(F)
	{
		setArraySize(F.numElements() + 1);
	}

	int getRoot(int *state) const
	{
		for (int i = 1; i <= F.numElements(); i++)
			state[i] = 0;
		return (F.numSets());
	}

	// state配列の更新と枝刈り
	bool updateStateWithPruning(int *state, int level) const
	{
		std::vector<int> sl = F.getSet(level + 1);

		for (int elem : sl)
		{
			if (state[elem] == 0)
				state[elem] = 1;
			else
				return (true);
		}
		return (false);
	}

	// フロンティアの更新
	void updateFrontier(int *state, int *frontier, int current_level, int level, int n) const
	{
		for (int i = 1; i <= n; i++)
		{
			if (state[i] == 1 || state[i] == -1)
				frontier[i] = 1;
			else
				frontier[i] = 0;
		}

		for (int i = 1; i < level; i++)
		{
			for (int elem : F.getSet(current_level + i + 1))
				if (frontier[elem] == 0)
					frontier[elem] = 2;
		}
	}

	// フロンティアによる枝刈り
	bool pruningByFrontier(int *state, int *frontier, int n) const
	{
		for (int i = 1; i <= n; i++)
		{
			if (frontier[i] == 0)
				return (true);
			else if (frontier[i] == 1)
				state[i] = -1;
		}
		return (false);
	}

	// 解の判定
	bool isCorrect(int *state, int n) const
	{
		int cnt = 0;

		for (int i = 1; i <= n; i++)
			if (state[i] == 1 || state[i] == -1)
				cnt++;

		if (cnt == n)
			return (true);
		return (false);
	}

	int getChild(int *state, int level, int value) const
	{
		int n = F.numElements();
		int m = F.numSets();
		int frontier[n + 1];
		int current_level = m - level;

		if (value)
			if (updateStateWithPruning(state, current_level))
				return (PRUNING);

		updateFrontier(state, frontier, current_level, level, n);

		if (pruningByFrontier(state, frontier, n))
			return (PRUNING);

		if (isCorrect(state, n))
			return (SUCCESS);

		return (level - 1);
	}
};

class MinElement : public tdzdd::DdEval<MinElement, int>
{
	int const n;
	int const *c;

public:
	MinElement(int n, int *c) : n(n), c(c) {}

	void evalTerminal(int &val, bool one) const
	{
		val = one ? 0 : INT_MAX / 2;
	}

	void evalNode(int &val, int level, tdzdd::DdValues<int, 2> const &values) const
	{
		val = std::min(values.get(0), values.get(1) + c[n - level]);
	}
};

class MaxElement : public tdzdd::DdEval<MaxElement, int>
{
	int const n;
	int const *c;

public:
	MaxElement(int n, int *c) : n(n), c(c) {}

	void evalTerminal(int &val, bool one)
	{
		val = one ? 0 : INT_MAX / 2;
	}

	void evalNode(int &val, int level, tdzdd::DdValues<int, 2> const &values) const
	{
		int max = std::max(values.get(1), c[n - level]);
		val = std::min(values.get(0), max);
	}
};

void outputSolutions(tdzdd::DdStructure<2> dd, FamilyofSets F)
{
	int m = F.numSets();
	int i = 1;

	std::cout << "解の列挙: " << std::endl;
	for (auto it = dd.begin(); it != dd.end(); ++it)
	{
		/* 実行可能解を１行ずつ出力 */
		std::cout << i << ": ";
		for (auto itr = (*it).begin(); itr != (*it).end(); ++itr)
			std::cout << m - *itr + 1 << " ";
		i++;
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

int main(int argc, char **argv)
{
	FamilyofSets F = readSets(argc, argv);
	SetZDD sets(F);
	tdzdd::DdStructure<2> dd(sets);
	int m = F.numSets();
	int n = F.numElements();
	int c[m];

	for (int i = 1; i <= m; i++)
		c[i - 1] = F.getWeight(i);

	// F.print();
	// 1: 解の個数を出力
	std::cout << "解の個数: " << dd.zddCardinality() << std::endl;
	std::cout << std::endl;

	// 2: 全ての集合分割を出力
	outputSolutions(dd, F);

	// 3: 重みの総和が最小の集合分割の重みの総和を出力
	std::cout << "重みの総和が最小の集合分割の重みの総和: " << dd.evaluate(MinElement(m, c)) << std::endl;
	std::cout << std::endl;

	// 4: 最大重みが最小の集合分割の最大重みの出力
	std::cout << "最大重みが最小の集合分割の最大重み: " << dd.evaluate(MaxElement(m, c)) << std::endl;
}
