#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <tdzdd/DdSpec.hpp>
#include <tdzdd/DdStructure.hpp>
#include <tdzdd/DdEval.hpp>
#include <tdzdd/DdSpecOp.hpp>

int cnt = 0;

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

	// 枝刈り
	bool isPruning(int *state, int level) const
	{
		int e = F.numElements();

		// 要素の重なり
		for (int i = 1; i <= e; i++)
			if (state[i] > 1)
				return (true);
		return (false);
	}

	// state配列の更新
	void updateState(int *state, int level) const
	{
		std::vector<int> sl = F.getSet(level + 1);

		for (int elem : sl)
			state[elem] += 1;
	}

	// 解の判定
	bool isCorrect(int *state) const
	{
		int e = F.numElements();

		for (int i = 1; i <= e; i++)
			if (state[i] != 1)
				return (false);
		return (true);
	}

	int getChild(int *state, int level, int value) const
	{
		// int e = F.numElements();
		int s = F.numSets();

		int current_level = s - level;

		if (value)
		{
			if (isPruning(state, level))
				return (0);
			updateState(state, current_level);
		}

		if (isCorrect(state))
			return (-1);

		return (level - 1);
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
		val = one ? 0 : INT_MAX;
	}

	void evalNode(int &val, int level, tdzdd::DdValues<int, 2> const &values) const
	{
		std::cout << values.get(0) << ":" << values.get(1) << ":" << c[n - level] << std::endl;
		val = std::max(values.get(0), values.get(1) + c[n - level]);
		// std::cout << "val:" << val << std::endl;
	}
};

int main(int argc, char **argv)
{
	FamilyofSets F = readSets(argc, argv);
	SetZDD sets(F);
	tdzdd::DdStructure<2> dd(sets);

	F.print();
	std::cout << "解の個数 : " << dd.zddCardinality() << std::endl;
}
