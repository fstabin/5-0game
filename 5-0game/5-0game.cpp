#include <unordered_map>
#include <queue>
#include <vector>
#include "..\..\acs\acs\include\hash.h"

class Hand {
	unsigned char l, r;
public:
	Hand(unsigned char al, unsigned char ar) {
		set(al, ar);
	}

	void set(unsigned char al, unsigned char ar) {
		if (al > ar) {
			l = al;
			r = ar;
		}
		else {
			l = ar;
			r = al;
		}
	}

	unsigned char getL()const{
		return l;
	}

	unsigned char getR() const{
		return r;
	}

	bool operator == (const Hand& h)const {
		return l == h.l && r == h.r;
	};
};

struct State {
	Hand A;
	Hand B;

	bool operator == (const State& r)const {
		return A == r.A && B == r.B;
	};
};

struct Prop {
	std::vector<State> from;			//遷移元ノード
	std::vector<State> to;				//遷移可能ノード
	uint32_t unconfirmedNode = 0;//まだ数値が確定していない遷移可能ノードの数
	uint32_t nim = std::numeric_limits<uint32_t>::max();
};

int main()
{
	State begStat = { Hand(1,1),Hand(1,1) };
	std::unordered_map<State, Prop, acs::hash::FNV_1<State> >tree;
	std::queue<State>que;

	//ツリーに存在するか
	//ツリーに存在しないとき新規ステートキューに追加
	//遷移できる状態を

	//ノードをつなぐ
	auto ConbineNode = [&que, &tree](State from, State to)->void{
		auto res = tree.insert(std::pair<State, Prop>(to, Prop()));
		//ツリーに遷移先ステートがが存在しないとき新規ステートキューに追加
		if (false == res.second) {
			que.push(to);
		}
		//ノードを有向グラフでつなぐ
		Prop& rpFrom = tree[from];
		++rpFrom.unconfirmedNode;
		rpFrom.to.push_back(to);

		Prop& rpTo = res.first->second;
		rpTo.from.push_back(to);
	};

	que.push(begStat);
	while (que.size()) {
		State s = que.front();
		unsigned char hv[4][3] = {
		{ s.A.getL(),  s.B.getL(),  s.B.getR() },
		{ s.A.getL(),  s.B.getR(),  s.B.getL() },
		{ s.A.getR(),  s.B.getL(),  s.B.getR() },
		{ s.A.getR(),  s.B.getR(),  s.B.getL() } };
		for (size_t i = 0; i < 4; i++)
		{
			auto mem = hv[i];
			if (mem[0] != 0 && mem[1] != 0)ConbineNode(s, { { Hand((mem[0] + mem[1]) % 5, mem[2]) },s.A });
		}
		que.pop();
	}

	for (unsigned char i = 0; i < 5; i++)
	{
		for (unsigned char j = 0; j < i + 1; j++)
		{
			State s = { { 0, 0 }, { i,j } };
			tree[s].nim = 0;
			que.push(s);
		}
	}

	//製作中
	while (que.size()) {
		State s = que.front();
		for (auto bs: s.from) {
			if(0 == --tree[bs].unconfirmedNode)que.push(bs);
		}
		que.pop();
	}
    return 0;
}

