#include <iostream>
#include <cstdlib>
#include <ctime>
#include "btree.hpp"

struct Bint100 {
	int key;
	int trash[100];
	Bint100(int x) {
		key = x;
	}
	Bint100(){}
	bool operator < (const Bint100& c) const {
		return (key ^ 1111) < (c.key ^ 1111);
	}
	void operator = (const int& x) {
		key = x;
	}
	bool operator == (const Bint100& c) const {
		return key == c.key;
	}
	bool operator != (const Bint100& c) const {
		return key != c.key;
	}
};

struct Bint1000 {
	int key;
	int trash[1000];
	Bint1000(int x) {
		key = x;
	}
	Bint1000(){}
	void operator = (const int& x) {
		key = x;
	}
	bool operator != (const Bint1000& c) {
		return key != c.key;
	}
};

sjtu::BTree<Bint100, Bint1000> btree;

Bint1000 func(Bint100 x) {
	int f = (x.key ^ 1111) << 3 + (x.key & 1111);
	if (x.key % 10) f = 0;
	Bint1000 Bf = f;
	return Bf;
}

int main() {
	srand(time(0));
	for (int i = 0 ; i < 1000; i ++) {
		Bint100 k = i;
		Bint1000 v = func(k);
		btree.insert(k, v);
	}
	for (int i = 0; i < 100; i++) {
		Bint100 k = rand() % 1000;
		Bint1000 v = btree.at(k);
		if (v != func(k)) {
			std::cout << "WA at query" << std::endl;
			return 0;
		}
	}
}