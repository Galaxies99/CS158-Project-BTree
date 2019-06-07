# include <iostream>
# include <stdlib.h>
# include <algorithm>
# include "BTree.hpp"
# include <vector>
using namespace std;

sjtu :: BTree <int, int> T;
vector<int> v;

int main() {
//	cerr << "begin insert: \n";
	for (int i=1, t; i<=10000000; ++i) {
		t = 1ll * rand() * rand() % 10000000;
		v.push_back(t);
		T.insert(t, i);
		if(i % 10000 == 0) cerr << i << endl;
//		cerr << i << endl;
	}
//	cerr << "end insert.\n";
	/*
	cerr << "begin erase: \n";
	for (int i=1; i<=250000; ++i) {
		int p = 4 * i - rand() % 4;
		if(T.erase(p) == sjtu :: Fail) throw "233";
		if (i % 10000 == 0) cerr << i << endl;
	}*/
//	T.debug_traverse();
//	for (int i=0; i<v.size(); ++i) {
//		T.erase(v[i]);
//		cerr << i << endl;
//	}
}