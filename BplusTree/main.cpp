# include <bits/stdc++.h>
# include <stdlib.h>
# include "utility.hpp"
# include <string.h>
# include <algorithm>
# include <windows.h>
# include "BTree.hpp"

using namespace std;

int data[20010], rd[20010];
int data2[1000010];

int main() {
	cout << "This is a very basic test code for your B (plus) Tree by Galaxies.\n";
	cout << "Version 1.0\n";
	cout << "\n";
	system("del btree");
	sjtu::BTree<int,int> T;
	for (int i=1; i<=20000; ++i) data[i] = i;
	random_shuffle(data+1, data+20001);
	for (int i=1; i<=20000; ++i) rd[data[i]] = i;
	cout << "  1. Insertion Test (Small) ...       ";
	for (int i=1; i<=20000; ++i) T.insert(i, data[i]);
	cout << " PASS!\n";
	cout << "  2. At Test (Small) ...              ";
	for (int i=1; i<=20000; ++i) {
		if(T.at(i) != data[i]) {
			cout << " FAIL!\n";
			cout << "Wrong at query " << i << endl;
			cout << "Your answer is " << T.at(i) << ", but the correct answer is " << data[i] << endl;
			return 0;
		}
	}
	cout << " PASS!\n";
	cout << "  3. Clear Test ...                    ";
	T.clear();
	if(T.size() != 0) {
		cout <<" FAIL!\n";
		return 0;
	} else cout << "PASS!\n";
	cout << "  4. Insertion Test After Clear ...   ";
	for (int i=1; i<=20000; ++i) T.insert(i, data[i]);
	cout << " PASS!\n";
	cout << "  5. At Test After Clear ...          ";
	for (int i=1; i<=20000; ++i) {
		if(T.at(i) != data[i]) {
			cout << " FAIL!\n";
			cout << "Wrong at query " << i << endl;
			cout << "Your answer is " << i << ", but the correct answer is " << data[i] << endl;
			return 0;
		}
	}
	cout << " PASS!\n";
	T.clear();
	cout << "  6. Random Insertion Test ...        ";
	for (int i=1; i<=20000; ++i) {
		T.insert(data[i], i);
	}
	cout << " PASS!\n";
	cout << "  7. At After Random Insertion ...    ";
	for (int i=1; i<=20000; ++i) {
		if(T.at(i) != rd[i]) {
			cout << " FAIL!\n";
			cout << "Wrong at query " << i << endl;
			cout << "Your answer is " << T.at(i) << ", but the correct answer is " << rd[i] << endl;
			return 0;
		}
	}
	cout << " PASS!\n";
	cout << "  8. Empty Test ...                   ";
	if(T.empty()) {
		cout << " FAIL!\n";
		return 0;
	}
	T.clear();
	if(!T.empty()) {
		cout << " FAIL!\n";
		return 0;
	}
	cout << " PASS!\n";
	cout << "  9. Big Random Insertion Persistent Test ... \n";
	for (int i=1; i<=20000; ++i) {
		T.insert(rand() * rand() * rand(), i);
	}
	cout << "PASS";


	return 0;
}