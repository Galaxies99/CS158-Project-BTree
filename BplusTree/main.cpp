# include <iostream>
# include <stdlib.h>
# include <algorithm>
# include "BTree.hpp"

using namespace std;

sjtu :: BTree <int, int> T;
int p[100001];
int d[10000001];

int main() {
	for (int i=1; i<=10000; ++i) p[i] = i;
	random_shuffle(p+1, p+10001);
	cerr << "begin insert: \n";
	for (int i=1; i<=10000; ++i) {
		T.insert(p[i], i); d[p[i]] = i;
		cerr << i << endl;
	}
	for (int i=1; i<=100; ++i) {
		cout << T.at(i + 9900) << endl;
	}
	cerr << "end insert.\n";
	/*
	cerr << "begin erase: \n";
	for (int i=1; i<=250000; ++i) {
		int p = 4 * i - rand() % 4;
		if(T.erase(p) == sjtu :: Fail) throw "233";
		if (i % 10000 == 0) cerr << i << endl;
	}*/
	T.debug_traverse();
	system("pause");
	T.debug_iterator();
	system("pause");
	T.debug_const_iterator();
//	for (int i=1; i<=2000; ++i) cout << d[i] << ' ';
	cout << endl;
//	T.debug_traverse();
//	sjtu :: BTree<int,int> T2(T);
//	T2.debug_traverse();
}