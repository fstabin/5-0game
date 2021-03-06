//5-0ゲーム(定義は後述)の解析プログラム
//・解析結果をファイルに出力
//・最適な行動の検索
//を行うことができる。

//5-0ゲーム定義
//プレイヤーA,Bには手が2本あり,指の数がgFingerAmount本ある
//はじめそれぞれ手ごとにゆびが1本ずつ立っている。
//現在攻撃側のプレイヤーは以下の行動がとれる
	//自分と相手の指が１本以上たっている手を、それぞれ１つずつ選択し、選択した相手の手の立っている指の数に、選択した自分の手の立っている数を足す。立っている指の数が指の数以上のとき(valueCutルール適応時、指をすべてたたむ。そうでない時、立っている指の数　%= 指の数)
	//(enableShareルール適応時)自分の立っている指の合計本数を維持したまま、それぞれの手の立っている指の数を変更する。ただし、それぞれの手で立っている指は１本以上、指の数未満とする。
//一回行動したら攻守を交代する
//どちらかのプレイヤーの立っている指の数が0になったらそのプレイヤーの負け

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <bitset>
#include <iostream>
#include <fstream>
#include "..\..\acs\acs\include\hash.h"

static const bool exportFile = true;//ファイルを出力するかどうか
static const bool enableCheck = true;//最適な行動の検索機能をオンにするかどうか

static const unsigned char gFingerAmount = 5;//指の本数
static const bool enableShare = true;//左右で指を共有できるか
static const bool valueCut = true;//立っている指が指の本数を超えたとき-true:値をゼロにする,false:(値) = (立っている指) % (指の本数)

static const uint32_t nim_invalid = std::numeric_limits<uint32_t>::max();//無効なnim数

class Hand {
	unsigned char l, r;
public:
	Hand(unsigned char al, unsigned char ar) {
		set(al, ar);
	}

	//立っている指の数2つをセットする
	void set(unsigned char al, unsigned char ar) {
		if (valueCut) {
			//立ってもよい指の数以上の時ゼロにする
			if (al >= gFingerAmount)al = 0;
			if (ar >= gFingerAmount)ar = 0;
		}
		else {
			//立ってもよい指の数以上の時あまりの数だけ立てる
			al %= gFingerAmount;
			ar %= gFingerAmount;
		}
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

	bool operator != (const Hand& h)const {
		return !(*this == h);
	};
};

struct State {
	Hand A;//現在のプレイヤー
	Hand B;//現在でないほう

	bool operator == (const State& r)const {
		return A == r.A && B == r.B;
	};
};

struct Prop {
	uint32_t loseNode = 0;//まだ数値が確定していない遷移可能ノードの数
	uint32_t nim = nim_invalid;//勝敗判定数(Nim数が出せないので0がまけ1が勝ちを表すのみ)
	std::unordered_set<State, acs::hash::FNV_1<State> > from;			//遷移元ノード
	std::unordered_set<State, acs::hash::FNV_1<State> > to;				//遷移可能ノード
};

int main()
{
	State begStat = { Hand(1,1),Hand(1,1) };//ゲームの初期状態
	std::unordered_map<State, Prop, acs::hash::FNV_1<State> >tree;//ゲーム木
	std::queue<State>que;


	//ゲーム木生成------------------------------------------

	//ツリーに存在するか
	//ツリーに存在しないとき新規ステートキューに追加
	//遷移できる状態を分部をつなぐ

	//ノードをつなぐ
	auto ConbineNode = [&que, &tree](State from, State to)->void {
		if (from == to)return;
		auto res = tree.insert(std::pair<State, Prop>(to, Prop()));
		//ツリーに遷移先ステートがが存在しないとき新規ステートキューに追加
		if (true == res.second) {
			que.push(to);
		}
		//ノードを有向グラフでつなぐ
		//遷移前のノード
		Prop& rpFrom = tree[from];
		if (true == rpFrom.to.insert(to).second) {
			++rpFrom.loseNode;
			//遷移後のノード
			Prop& rpTo = res.first->second;
			rpTo.from.insert(from);
		}
	};

	tree[begStat] = Prop();
	que.push(begStat);
	while (que.size()) {
		State& s = que.front();
		//敵の指をたたく変化
		unsigned char hv[4][3] = {
		{ s.A.getL(),  s.B.getL(),  s.B.getR() },
		{ s.A.getL(),  s.B.getR(),  s.B.getL() },
		{ s.A.getR(),  s.B.getL(),  s.B.getR() },
		{ s.A.getR(),  s.B.getR(),  s.B.getL() } };
		for (size_t i = 0; i < 4; i++)
		{
			auto mem = hv[i];
			if (mem[0] != 0 && mem[1] != 0)ConbineNode(s, { { Hand((mem[0] + mem[1]), mem[2]) },s.A });
		}
		//自分の指を変える変化
		if (enableShare) {
			if (s.A != Hand(0, 0) && s.B != Hand(0, 0)) {
				//0 <= hr <= |Aの立ってる指の和|
				for (uint32_t hl = 0; hl <= s.A.getL() + s.A.getR(); hl++)
				{
					uint32_t hr = s.A.getL() + s.A.getR() - hl;
					if (hl < hr)continue;//常に左手のほうが大きい
					if (hl >= gFingerAmount || hr >= gFingerAmount)continue;//立っている指はgFingerAmount本未満
					if (hl == 0 || hr == 0)continue;//立っている指は一本以上
					Hand ch(hl, hr);
					if (ch == s.A)continue;//変化しないのは禁止
					ConbineNode(s, { s.B,ch });
				}
			}
		}
		que.pop();
	}

	//ゲーム木生成 終了------------------------------------------

	//解析---------------------------------------------

	//nimを確定する
	auto fixNim = [&](State& s, uint32_t nim)->void {
		Prop& p = tree[s];
		if (p.nim == nim_invalid) {
			p.nim = nim;
			que.push(s);
		}
	};
	for (unsigned char i = 0; i < gFingerAmount; i++)
	{
		for (unsigned char j = 0; j < i + 1; j++)
		{
			State s = { Hand(0, 0), Hand(i,j) };
			fixNim(s, 0);
		}
	}
	while (que.size()) {
		State& s = que.front();
		//nimを確定
		Prop& p = tree[s];
		//負け状態に遷移できるとき　現在の状態は勝ち
		if (p.loseNode > 0) {
			//この状態の遷移元ノードの敗北子ノードカウントを減少させる
			for (auto bs : p.from) {
				//敗北の子ノードがないとき遷移元ノードは負けノード確定
				if (0 == --tree[bs].loseNode) {
					fixNim(bs, 0);
				}
			}
		}
		//負け状態に遷移できないとき　現在の状態はまけ
		else {
			//この状態に遷移元ノードはすべて勝ちに確定する
			for (auto bs : p.from) {
				fixNim(bs, 1);
			}
		}
		que.pop();
	}

	//解析終了---------------------------------------------

	//結果を出力-------------------------------------
	if (exportFile) {
		//出力ファイル名は "(指の数)" + "-0game_result" + "-(状態) + ".csv"
			//状態{s, c}
		std::string fileName = std::to_string(gFingerAmount);
		fileName += "-0game_result";
		if (enableShare)fileName += "-s";
		if (valueCut)fileName += "-c";
		std::ofstream out(fileName + ".csv");

		//形式はコンマ区切りのcsv
		//縦　現在攻撃側のプレイヤー
		//横　現在防御側のプレイヤー
		//0 負け　1 勝ち　N/A 不明

		out << "x";
		for (unsigned int a = 0; a < gFingerAmount; a++)
		{
			for (unsigned int b = 0; b < a + 1; b++)
			{
				out << "," << a << "/" << b;
			}
		}
		out << std::endl;
		for (unsigned int i = 0; i < gFingerAmount; i++)
		{
			for (unsigned int j = 0; j <= i; j++)
			{
				out << i << "/" << j;
				for (unsigned int a = 0; a < gFingerAmount; a++)
				{
					for (unsigned int b = 0; b <= a; b++)
					{
						State s = { Hand(i, j), Hand(a,b) };
						if (tree[s].nim == nim_invalid)out << "," << "N/A";
						else out << "," << tree[s].nim;
					}
				}
				out << std::endl;
			}
		}
	}
	//結果を出力 終了-------------------------------------

	//最適な行動の検索 -------------------------------------

	//入出力の記号
		//a 現在攻撃側のプレイヤーの状態
		//b 現在防御側のプレイヤーの状態
		//A 現在攻撃側のプレイヤーの次の状態
		//B 現在防御側のプレイヤーの次の状態
		//l,r 左右の手のどちらかの立っている指の数
		//s 現在の確定勝敗(win:勝利確定, N/A:未確定)
	//入力
		//a.l a.r b.l b.r
	//出力
		//確定で負けでないとき
		//A.l A.r B.l B.r s
		//確定で負けの時
		//lose

	if (enableCheck) {
		unsigned int al, ar, bl, br;
		using std::cin;
		using std::cout;
		using std::endl;
		while (std::cin >> al >> ar >> bl >> br)
		{
			auto& v = tree[{Hand(al, ar), Hand(bl, br)}].to;
			for (auto& p : v)
			{
				if (tree[p].nim == 0) {
					cout << (int)p.B.getL() << " " << (int)p.B.getR() << " " << (int)p.A.getL() << " " << (int)p.A.getR() << " win";
					goto NEXTLOOP;
				}
			}
			for (auto& p : v)
			{
				if (tree[p].nim == nim_invalid) {
					cout << (int)p.B.getL() << " " << (int)p.B.getR() << " " << (int)p.A.getL() << " " << (int)p.A.getR() << " N/A";
					goto NEXTLOOP;
				}
			}
			cout << "lose";
		NEXTLOOP:
			{
				cout << endl;
			}
		}
	}
	//最適な行動の検索 終了-------------------------------------
	
	return 0;
}

