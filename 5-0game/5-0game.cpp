#include <unordered_map>
#include <queue>
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

	unsigned char getL() {
		return l;
	}

	unsigned char getR() {
		return r;
	}
};

struct State {
	Hand A;
	Hand B;
};

struct Node {
	std::vector<State>from;
	std::vector<State>to;
	uint32_t ref;
	uint32_t nim;
};

int main()
{

	State begStat = { Hand(1,1),Hand(1,1) };
	std::unordered_map<State, Node, acs::hash::FNV_1<State> >tree;
	std::queue<State>que;

	//ツリーに存在するか
	//ツリーに存在しないとき新規ステートキューに追加
	//遷移できる状態を

	auto iter = tree.find(begStat);
	if (iter == tree.end()){
		que.push({ Hand(1,1),Hand(1,1) });
	}

	
	while (que.size()) {


		que.pop();
	}
    return 0;
}

