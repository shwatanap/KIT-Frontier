#include <tdzdd/DdSpec.hpp>
#include <tdzdd/DdStructure.hpp>
#include <tdzdd/DdEval.hpp>

#include <vector>
#include <fstream>
#include <iostream>

#define FREE -1

typedef enum e_frontier_type
{
    NOT,
    CURRENT,
    NEXT,
} t_frontier_type;

// グラフを表すクラス Graph
class Graph
{
private:
    int const s;
    int const t;
    int const n;
    int const m;
    std::vector<std::pair<int, int>> const edge_list;

public:
    Graph(int s, int t, int n, int m, std::vector<std::pair<int, int>> edge_list)
        : s(s), t(t), n(n), m(m), edge_list(edge_list)
    {
    }

    std::pair<int, int> getEdge(int e) const
    {
        return std::make_pair(edge_list[e].first, edge_list[e].second);
    }

    int getStart() const
    {
        return s;
    }

    int getTerminal() const
    {
        return t;
    }

    int numVertices() const
    {
        return n;
    }

    int numEdges() const
    {
        return m;
    }

    void printEdges() const
    {
        // std::cout << "#edges: " << m << std::endl;
        for (auto it = edge_list.begin(); it != edge_list.end(); ++it)
        {
            std::cout << (*it).first << ", " << (*it).second << std::endl;
        }
    }

    void print() const
    {
        std::cout << "#Vertices: " << numVertices() << std::endl;
        std::cout << "#Edges: " << numEdges() << std::endl;
        std::cout << "Start, Terminal = " << s << ", " << t << std::endl;
        std::cout << "Edge List: " << std::endl;
        printEdges();
    }
};

// 入力ファイルの読み込み
Graph readGraph(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Please input a graph file" << std::endl;
        exit(-1);
    }
    std::ifstream ifs(argv[1]);
    if (!ifs)
    {
        std::cerr << "Error: cannot open file " << argv[1] << std::endl;
        exit(FREE);
    }

    int s, t;
    if (ifs)
        ifs >> s >> t;

    std::vector<std::pair<int, int>> edge_list;
    int u, v;
    int n = 0;
    while (ifs >> u >> v)
    {
        edge_list.push_back(std::make_pair(u, v));
        if (n < u)
            n = u;
        if (n < v)
            n = v;
    }
    Graph G(s, t, n, edge_list.size(), edge_list);

    return G;
}

// TdZdd で表現すべき部分
class PathZDD : public tdzdd::PodArrayDdSpec<PathZDD, int, 2>
{
    Graph const G;

public:
    PathZDD(Graph G) : G(G)
    {
        setArraySize(G.numEdges() + 1);
    }

    int getRoot(int *mate) const
    {
        for (int i = 1; i < G.numVertices() + 1; i++)
            mate[i] = i;
        return (G.numEdges());
    }

    void printMate(int *mate) const
    {
        printf("--- mate ---\n");
        for (int i = 1; i < G.numVertices() + 1; i++)
            printf("%d, %d\n", i, mate[i]);
        printf("------------\n");
    }

    void updateMate(int *mate, int current_path) const
    {
        std::pair<int, int> side_ends = G.getEdge(current_path);
        int u = side_ends.first;
        int v = side_ends.second;
        int w = mate[u];
        int x = mate[v];
        // 1. 新しいペアの出現
        if (mate[u] == u && mate[v] == v)
        {
            mate[u] = v;
            mate[v] = u;
        }
        // 2. 既存のパスの延長
        // どっちかいらない？？
        else if (mate[v] == v && mate[w] == u && mate[u] == w)
        {
            mate[v] = w;
            mate[w] = v;
            mate[u] = FREE;
        }
        else if (mate[u] == u && mate[x] == v && mate[v] == x)
        {
            mate[u] = x;
            mate[x] = u;
            mate[v] = FREE;
        }
        // 3. 2つのパスの融合
        else if (mate[u] == w && mate[w] == u && mate[v] == x && mate[x] == v)
        {
            mate[w] = x;
            mate[x] = w;
            mate[u] = FREE;
            mate[v] = FREE;
        }
    }

    bool isCorrect(int *mate) const
    {
        int s = G.getStart();
        int t = G.getTerminal();

        if (!(mate[s] == t && mate[t] == s))
            return (false);
        for (int i = 2; i < G.numVertices(); i++)
            if (!(mate[i] == FREE || mate[i] == i))
                return (false);
        // printMate(mate);
        return (true);
    }

    // 枝刈りを判定を行う関数
    bool isPruning(int *mate, int current_path) const
    {
        std::pair<int, int> side_ends = G.getEdge(current_path);
        int u = side_ends.first;
        int v = side_ends.second;
        int s = G.getStart();
        int t = G.getTerminal();

        // 1. sが中点
        if (mate[s] == FREE || mate[t] == FREE)
            return (true);
        // 2. sが孤立かつsに新しい辺が作られる可能性がない場合
        if (mate[s] == s && u != s)
            return (true);
        // 3. sとt以外の頂点が端点となってフロンティアから抜ける
        // 4. パスの中点に接続
        if (mate[u] == FREE || mate[v] == FREE)
            return (true);
        // 5. 平路が生じる
        if (mate[u] == v && mate[v] == u)
            return (true);
        return (false);
    }

    bool isFrontierPruning(int *mate, int current_path) const
    {
        int v_num = G.numVertices();
        int e_num = G.numEdges();
        e_frontier_type frontier[v_num];

        // frontierの初期化
        for (int i = 0; i < v_num; i++)
            frontier[i] = NOT;
        for (int i = current_path; i <= e_num; i++)
        {
            std::pair<int, int> side_ends = G.getEdge(i);
            if (i == current_path)
            {
                frontier[side_ends.first] = CURRENT;
                frontier[side_ends.second] = CURRENT;
            }
            else
            {
                frontier[side_ends.first] = NEXT;
                frontier[side_ends.second] = NEXT;
            }
        }

        std::pair<int, int> side_ends = G.getEdge(current_path);
        int u = side_ends.first;
        int v = side_ends.second;
        for (int i = 2; i < v_num; i++)
        {
            if (frontier[i] == CURRENT)
            {
                if (i == u)
                    if (mate[u] != u && mate[u] != FREE)
                        return (true);
                if (i == v)
                    if (mate[v] != v && mate[v] != FREE)
                        return (true);
                mate[i] = FREE;
            }
        }
        return (false);
    }

    int getChild(int *mate, int level, int value) const
    {
        int current_path = G.numEdges() - level;
        int v_num = G.numVertices();
        int e_num = G.numEdges();

        if (value)
        {
            // 枝刈り
            if (isPruning(mate, current_path))
                return (0);
            // mateの更新
            updateMate(mate, current_path);
        }
        if (isFrontierPruning(mate, current_path))
            return (0);
        // 解が完成ならFREEを返す
        if (isCorrect(mate))
            return (-1);
        // 何も起こらない場合 level - 1を返す
        return (level - 1);
    }
};

void solutionOutput(tdzdd::DdStructure<2> dd, Graph G)
{
    for (auto &path_set : dd)
    {
        for (int i = 0; i < G.numEdges(); ++i)
        {
            int level = G.numEdges() - i;
            if (path_set.count(level) == 1)
                std::cout << G.getEdge(i).first << " " << G.getEdge(i).second << std::endl;
        }
        std::cout << std::endl;
    }
}

int main(int argc, char **argv)
{
    Graph G = readGraph(argc, argv);
    G.print();
    PathZDD path(G);
    tdzdd::DdStructure<2> dd(path);

    // 解の出力
    solutionOutput(dd, G);

    // 解の数の出力
    std::cout << "解の数: " << dd.zddCardinality() << std::endl;

    // グラフ出力
    // dd.dumpDot();
}
