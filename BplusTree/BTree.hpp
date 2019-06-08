# include "utility.hpp"
# include <fstream>
# include <functional>
# include <cstddef>
# include "exception.hpp"

namespace sjtu {

	int ID = 0;
	template <class KeyType, class ValueType, class Compare = std::less<KeyType> >
	class BTree {
		public:
			typedef pair <KeyType, ValueType> value_type;
			typedef ssize_t node_t;
			typedef ssize_t offset_t;

			class iterator;

			class const_iterator;

		private:
			static const int M = (4079 / (sizeof(node_t) + sizeof(KeyType))) < 5 ? 4 : (4079 / (sizeof(node_t) + sizeof(KeyType)));                // need modify
			static const int L = (4076 / (sizeof(value_type))) < 5 ? 4 : (4076 / (sizeof(value_type)));                 // need modify
//			static const int M = 1000;
//			static const int L = 200;
			static const int MMIN = (M+1) / 2;            // M / 2
			static const int LMIN = (L+1) / 2;            // L / 2
			static const int info_offset = 0;

			struct nameString {
				char *str;

				nameString() { str = new char[10]; }

				~nameString() { if (str != nullptr) delete str; }

				void setName(int id) {
					if (id < 0 || id > 9) throw "no more B plus Tree!";
					str[0] = 'd';
					str[1] = 'a';
					str[2] = 't';
					str[3] = static_cast <char> (id + '0');
					str[4] = '.';
					str[5] = 'd';
					str[6] = 'a';
					str[7] = 't';
					str[8] = '\0';
				}

				void setName(char *_str) {
					int i = 0;
					for (; _str[i]; ++i) str[i] = _str[i];
					str[i] = 0;
				}
			};

			struct basicInfo {
				node_t head;          // head of leaf
				node_t tail;          // tail of leaf
				node_t root;          // root of Btree
				size_t size;          // size of Btree
				offset_t eof;         // end of file
				basicInfo() {
					head = 0;
					tail = 0;
					root = 0;
					size = 0;
					eof = 0;
				}
			};

			struct leafNode {
				offset_t offset;          // offset
				node_t par;               // parent
				node_t pre, nxt;          // previous and next leaf
				int cnt;                  // number of pairs in leaf
				value_type data[L + 1];   // data
				leafNode() {
					offset = 0, par = 0, pre = 0, nxt = 0, cnt = 0;
				}
			};
			struct internalNode {
				offset_t offset;      	// offset
				node_t par;           	// parent
				node_t ch[M + 1];     	// children
				KeyType key[M + 1];   	// key
				int cnt;              	// number in internal node
				bool type;            	// child is leaf or not
				internalNode() {
					offset = 0, par = 0;
					for (int i = 0; i <= M; ++i) ch[i] = 0;
					cnt = 0;
					type = 0;
				}
			};


			FILE *fp;
			bool fp_open;
			nameString fp_name;
			basicInfo info;

			// ================================= file operation ===================================== //
			/**
			 * Instructions:
			 *    openFile(): if file exists, open it; else create it and open it.
			 *    closeFile(): close file.
			 *    readFile(*place, offset, num, size): read size * num bytes from the offset position of the
			 *                                         file, then put it in *place. return successful operations.
			 *    writeFile(*place, offset, num, size): write size * num bytes to the offset position of the
			 *                                         file from *place, return successful operations.
			 *    copy_readFile(*place, offset, num, size): the same function as readFile, just different filename.
			 *    copy_leaf(offset, from_offset, par_offset): copy the leaf from from_offset to offset.
			 *    copy_node(offset, from_offset, par_offset): copy the internal node from from_offset to offset.
			 *    copy_File(name1, name2): copy file from name1 to name2.
			 */
			bool file_already_exists;

			inline void openFile() {
				file_already_exists = 1;
				if (fp_open == 0) {
					fp = fopen(fp_name.str, "rb+");
					if (fp == nullptr) {
						file_already_exists = 0;
						fp = fopen(fp_name.str, "w");
						fclose(fp);
						fp = fopen(fp_name.str, "rb+");
					} else readFile(&info, info_offset, 1, sizeof(basicInfo));
					fp_open = 1;
				}
			}

			inline void closeFile() {
				if (fp_open == 1) {
					fclose(fp);
					fp_open = 0;
				}
			}

			inline void readFile(void *place, offset_t offset, size_t num, size_t size) const {
				if (fseek(fp, offset, SEEK_SET)) throw "open file failed!";
				fread(place, size, num, fp);
			}

			inline void writeFile(void *place, offset_t offset, size_t num, size_t size) const {
				if (fseek(fp, offset, SEEK_SET)) throw "open file failed!";
				fwrite(place, size, num, fp);
			}

			nameString fp_from_name;
			FILE *fp_from;

			inline void copy_readFile(void *place, offset_t offset, size_t num, size_t size) const{
				if (fseek(fp_from, offset, SEEK_SET)) throw "open file failed";
				size_t ret = fread(place, num, size, fp_from);
			}

			offset_t leaf_offset_temp;

			inline void copy_leaf(offset_t offset, offset_t from_offset, offset_t par_offset) {
				leafNode leaf, leaf_from, pre_leaf;
				copy_readFile(&leaf_from, from_offset, 1, sizeof(leafNode));
				leaf.offset = offset, leaf.par = par_offset;
				leaf.cnt = leaf_from.cnt; leaf.pre = leaf_offset_temp; leaf.nxt = 0;
				if(leaf_offset_temp != 0) {
					readFile(&pre_leaf, leaf_offset_temp, 1, sizeof(leafNode));
					pre_leaf.nxt = offset;
					writeFile(&pre_leaf, leaf_offset_temp, 1, sizeof(leafNode));
					info.tail = offset;
				} else info.head = offset;
				for (int i=0; i<leaf.cnt; ++i) leaf.data[i].first = leaf_from.data[i].first, leaf.data[i].second = leaf_from.data[i].second;
				writeFile(&leaf, offset, 1, sizeof(leafNode));
				info.eof += sizeof(leafNode);
				leaf_offset_temp = offset;
			}

			inline void copy_node(offset_t offset, offset_t from_offset, offset_t par_offset) {
				internalNode node, node_from;
				copy_readFile(&node_from, from_offset, 1, sizeof(internalNode));
				writeFile(&node, offset, 1, sizeof(internalNode));
				info.eof += sizeof(internalNode);
				node.offset = offset; node.par = par_offset;
				node.cnt = node_from.cnt; node.type = node_from.type;
				for (int i=0; i<node.cnt; ++i) {
					node.key[i] = node_from.key[i];
					if(node.type == 1) {  					// leaf
						copy_leaf(info.eof, node_from.ch[i], offset);
					} else {                        // node
						copy_node(info.eof, node_from.ch[i], offset);
					}
				}
				writeFile(&node, offset, 1, sizeof(internalNode));
			}

			inline void copyFile(char *to, char *from) {
				fp_from_name.setName(from);
				fp_from = fopen(fp_from_name.str, "rb+");
				if (fp_from == nullptr) throw "no such file";
				basicInfo infoo;
				copy_readFile(&infoo, info_offset, 1, sizeof(basicInfo));
				leaf_offset_temp = 0; info.size = infoo.size;
				info.root = info.eof = sizeof(basicInfo);
				writeFile(&info, info_offset, 1, sizeof(basicInfo));
				copy_node(info.root, infoo.root, 0);
				writeFile(&info, info_offset, 1, sizeof(basicInfo));
				fclose(fp_from);
			}

			// ============================= end of file operation =================================== //

			/**
			 * function: build an tree with no elements.
			 */
			inline void build_tree() {
				info.size = 0;
				info.eof = sizeof(basicInfo);
				internalNode root;
				leafNode leaf;
				info.root = root.offset = info.eof;
				info.eof += sizeof(internalNode);
				info.head = info.tail = leaf.offset = info.eof;
				info.eof += sizeof(leafNode);
				root.par = 0; root.cnt = 1; root.type = 1;
				root.ch[0] = leaf.offset;
				leaf.par = root.offset;
				leaf.nxt = leaf.pre = 0;
				leaf.cnt = 0;
				writeFile(&info, info_offset, 1, sizeof(basicInfo));
				writeFile(&root, root.offset, 1, sizeof(internalNode));
				writeFile(&leaf, leaf.offset, 1, sizeof(leafNode));
			}

			/**
			 * function: given a key and find the leaf it should be in.
			 * return the offset of the leaf.
			 */
			node_t locate_leaf(const KeyType &key, offset_t offset) const {
				internalNode p;
				readFile(&p, offset, 1, sizeof(internalNode));
				if(p.type == 1) {
					// child -> leaf
					int pos = 0;
					for (; pos < p.cnt; ++pos)
						if (key < p.key[pos]) break;
					if (pos == 0) return 0;
					return p.ch[pos - 1];
				} else {
					int pos = 0;
					for (; pos < p.cnt; ++pos)
						if (key < p.key[pos]) break;
					if (pos == 0) return 0;
					return locate_leaf(key, p.ch[pos - 1]);
				}
			}

			/**
			 * function: insert an element (key, value) to the given leaf.
			 * return Fail and operate nothing if there are elements with same key.
			 * return Success if inserted.
			 * if leaf count is bigger than L then call split_leaf().
			 */
			pair <iterator, OperationResult> insert_leaf(leafNode &leaf, const KeyType &key, const ValueType &value) {
				iterator ret;
				int pos = 0;
				for (; pos < leaf.cnt; ++pos) {
					if (key == leaf.data[pos].first) return pair <iterator, OperationResult> (iterator(nullptr), Fail);			// there are elements with the same key
					if (key < leaf.data[pos].first) break;
				}
				for (int i = leaf.cnt - 1; i >= pos; --i)
					leaf.data[i+1].first = leaf.data[i].first, leaf.data[i+1].second = leaf.data[i].second;
				leaf.data[pos].first = key; leaf.data[pos].second = value;
				++leaf.cnt;
				++info.size;
				ret.from = this; ret.place = pos; ret.offset = leaf.offset;
				writeFile(&info, info_offset, 1, sizeof(basicInfo));
				if(leaf.cnt <= L) writeFile(&leaf, leaf.offset, 1, sizeof(leafNode));
				else split_leaf(leaf, ret, key);
				return pair <iterator, OperationResult> (ret, Success);
			}

			/**
			 * function: insert an key elements (only key) to the given internal node.
			 *           insert an child to the given internal node.
			 * notice: elements in child is bigger than key.
			 * if node count is bigger than M then call split_node().
			 */
			void insert_node(internalNode &node, const KeyType &key, node_t ch) {
				int pos = 0;
				for (; pos < node.cnt; ++pos)
					if (key < node.key[pos]) break;
				for (int i = node.cnt - 1; i >= pos; --i)
					node.key[i+1] = node.key[i];
				for (int i = node.cnt - 1; i >= pos; --i)
					node.ch[i+1] = node.ch[i];
				node.key[pos] = key;
				node.ch[pos] = ch;
				++node.cnt;
				if(node.cnt <= M) writeFile(&node, node.offset, 1, sizeof(internalNode));
				else split_node(node);
			}

			/**
			 * function: split a leaf into two parts.
			 * then, call insert_node() to insert a (key, ch) pair in the father node.
			 */
			void split_leaf(leafNode &leaf, iterator &it, const KeyType &key) {
				leafNode newleaf;
				newleaf.cnt = leaf.cnt - (leaf.cnt >> 1);
				leaf.cnt = leaf.cnt >> 1;
				newleaf.offset = info.eof;
				info.eof += sizeof(leafNode);
				newleaf.par = leaf.par;
				for (int i=0; i<newleaf.cnt; ++i) {
					newleaf.data[i].first = leaf.data[i + leaf.cnt].first, newleaf.data[i].second = leaf.data[i + leaf.cnt].second;
					if(newleaf.data[i].first == key) {
						it.offset = newleaf.offset;
						it.place = i;
					}
				}
				// link operation begin
				newleaf.nxt = leaf.nxt;
				newleaf.pre = leaf.offset;
				leaf.nxt = newleaf.offset;
				leafNode nxtleaf;
				if(newleaf.nxt == 0) info.tail = newleaf.offset;
				else {
					readFile(&nxtleaf, newleaf.nxt, 1, sizeof(leafNode));
					nxtleaf.pre = newleaf.offset;
					writeFile(&nxtleaf, nxtleaf.offset, 1, sizeof(leafNode));
				}
				// link operation end

				writeFile(&leaf, leaf.offset, 1, sizeof(leafNode));
				writeFile(&newleaf, newleaf.offset, 1, sizeof(leafNode));
				writeFile(&info, info_offset, 1, sizeof(basicInfo));

				// update father
				internalNode par;
				readFile(&par, leaf.par, 1, sizeof(internalNode));
				insert_node(par, newleaf.data[0].first, newleaf.offset);
			}

			/**
			 * function: split a node into two parts.
			 * then, call insert_node() to insert a (key, ch) pair in the father node.
			 */
			void split_node(internalNode &node) {
				internalNode newnode;
				newnode.cnt = node.cnt - (node.cnt >> 1);
				node.cnt >>= 1;
				newnode.par = node.par;
				newnode.type = node.type;
				newnode.offset = info.eof;
				info.eof += sizeof(internalNode);
				for (int i = 0; i < newnode.cnt; ++i)
					newnode.key[i] = node.key[i + node.cnt];
				for (int i = 0; i < newnode.cnt; ++i)
					newnode.ch[i] = node.ch[i + node.cnt];

				// updating children's parents
				leafNode leaf;
				internalNode internal;
				for (int i = 0; i < newnode.cnt; ++i) {
					if(newnode.type == 1) {  				// his child is leaf
						readFile(&leaf, newnode.ch[i], 1, sizeof(leafNode));
						leaf.par = newnode.offset;
						writeFile(&leaf, leaf.offset, 1, sizeof(leafNode));
					} else {
						readFile(&internal, newnode.ch[i], 1, sizeof(internalNode));
						internal.par = newnode.offset;
						writeFile(&internal, internal.offset, 1, sizeof(internalNode));
					}
				}

				if(node.offset == info.root) {				// root
					// new root
					internalNode newroot;
					newroot.par = 0;
					newroot.type = 0;
					newroot.offset = info.eof;
					info.eof += sizeof(internalNode);
					newroot.cnt = 2;
					newroot.key[0] = node.key[0];
					newroot.ch[0] = node.offset;
					newroot.key[1] = newnode.key[0];
					newroot.ch[1] = newnode.offset;
					node.par = newroot.offset;
					newnode.par = newroot.offset;
					info.root = newroot.offset;

					writeFile(&info, info_offset, 1, sizeof(basicInfo));
					writeFile(&node, node.offset, 1, sizeof(internalNode));
					writeFile(&newnode, newnode.offset, 1, sizeof(internalNode));
					writeFile(&newroot, newroot.offset, 1, sizeof(internalNode));
				} else {															// not root
					writeFile(&info, info_offset, 1, sizeof(basicInfo));
					writeFile(&node, node.offset, 1, sizeof(internalNode));
					writeFile(&newnode, newnode.offset, 1, sizeof(internalNode));

					internalNode par;
					readFile(&par, node.par, 1, sizeof(internalNode));
					insert_node(par, newnode.key[0], newnode.offset);
				}
			}

			/**
			 * function: borrow an element from right brother.
			 * return Success if success, return Fail if fail.
			 */
			OperationResult borrow_right(leafNode leaf) {
				if (leaf.nxt == 0) return Fail;                  // no next leaf
				leafNode right;
				readFile(&right, leaf.nxt, 1, sizeof(leafNode));
				if (leaf.par != right.par) return Fail;          // not brother
				if (right.cnt <= LMIN) return Fail;              // no more elements.

				// ok! Borrow from right!
				KeyType oldkey, newkey;
				oldkey = right.data[0].first;
				newkey = right.data[1].first;
				leaf.data[leaf.cnt].first = right.data[0].first;
				leaf.data[leaf.cnt].second = right.data[0].second;
				leaf.cnt++;
				right.cnt--;
				for (int i = 0; i < right.cnt; ++i) right.data[i].first = right.data[i + 1].first, right.data[i].second = right.data[i + 1].second;

				internalNode node;
				readFile(&node, leaf.par, 1, sizeof(internalNode));
				for (int i = 0; i < node.cnt; ++i) {
					if (node.key[i] == oldkey) {
						node.key[i] = newkey;
						break;
					}
				}

				writeFile(&node, node.offset, 1, sizeof(internalNode));
				writeFile(&leaf, leaf.offset, 1, sizeof(leafNode));
				writeFile(&right, right.offset, 1, sizeof(leafNode));
				return Success;
			}

			/**
			 * function: borrow an element from left brother.
			 * return Success if success, return Fail if fail.
			 */
			OperationResult borrow_left(leafNode leaf) {
				if (leaf.pre == 0) return Fail;                  // no previous leaf
				leafNode left;
				readFile(&left, leaf.pre, 1, sizeof(leafNode));
				if (leaf.par != left.par) return Fail;          // not brother
				if (left.cnt <= LMIN) return Fail;              // no more elements.

				// ok! Borrow from left!
				KeyType oldkey, newkey;
				oldkey = leaf.data[0].first;
				newkey = left.data[left.cnt - 1].first;
				for (int i = leaf.cnt - 1; i >= 0; -- i)
					leaf.data[i + 1].first = leaf.data[i].first, leaf.data[i + 1].second = leaf.data[i].second;
				leaf.data[0].first = left.data[left.cnt - 1].first;
				leaf.data[0].second = left.data[left.cnt - 1].second;
				++leaf.cnt;
				--left.cnt;

				internalNode node;
				readFile(&node, leaf.par, 1, sizeof(internalNode));
				for (int i = 0; i < node.cnt; ++i) {
					if (node.key[i] == oldkey) {
						node.key[i] = newkey;
						break;
					}
				}

				writeFile(&node, node.offset, 1, sizeof(internalNode));
				writeFile(&leaf, leaf.offset, 1, sizeof(leafNode));
				writeFile(&left, left.offset, 1, sizeof(leafNode));
				return Success;
			}

			/**
			 * function: merge with right brother.
			 * return Success if success, return Fail if fail.
			 */
			OperationResult merge_right(leafNode leaf) {
				if(leaf.nxt == 0) return Fail;
				leafNode right;
				readFile(&right, leaf.nxt, 1, sizeof(leafNode));
				if(right.par != leaf.par) return Fail;
				for (int i = 0; i < right.cnt; ++i) leaf.data[leaf.cnt].first = right.data[i].first, leaf.data[leaf.cnt].second = right.data[i].second, ++leaf.cnt;
				leaf.nxt = right.nxt;
				if(right.offset == info.tail) {
					info.tail = leaf.offset;
					writeFile(&info, info_offset, 1, sizeof(basicInfo));
				} else {
					leafNode temp;
					readFile(&temp, leaf.nxt, 1, sizeof(leafNode));
					temp.pre = leaf.offset;
					writeFile(&temp, temp.offset, 1, sizeof(leafNode));
				}

//				std :: cerr << "merge right...\n";

				internalNode node;
				readFile(&node, leaf.par, 1, sizeof(internalNode));
				int pos = 0;
				for (; pos < node.cnt; ++pos)
					if(node.key[pos] == right.data[0].first) break;
				for (int i = pos; i < node.cnt - 1; ++ i)
					node.key[i] = node.key[i + 1], node.ch[i] = node.ch[i + 1];
				node.cnt --;

				writeFile(&leaf, leaf.offset, 1, sizeof(leafNode));

				if(check_node(node) == Success) writeFile(&node, node.offset, 1, sizeof(internalNode));
				else operate_node(node);
				return Success;
			}

			/**
			 * function: merge with left brother.
			 * return Success if success, return Fail if fail.
			 */
			OperationResult merge_left(leafNode leaf) {
				if (leaf.pre == 0) return Fail;
				leafNode left;
				readFile(&left, leaf.pre, 1, sizeof(leafNode));
				if (left.par != leaf.par) return Fail;
				for (int i = 0; i < leaf.cnt; ++i) left.data[left.cnt].first = leaf.data[i].first, left.data[left.cnt].second = leaf.data[i].second, ++left.cnt;
				left.nxt = leaf.nxt;
				if(info.tail == leaf.offset) {
					info.tail = left.offset;
					writeFile(&info, info_offset, 1, sizeof(basicInfo));
				} else {
					leafNode temp;
					readFile(&temp, left.nxt, 1, sizeof(leafNode));
					temp.pre = left.offset;
					writeFile(&temp, temp.offset, 1, sizeof(leafNode));
				}

				internalNode node;
				readFile(&node, left.par, 1, sizeof(internalNode));
				int pos = 0;
				for (; pos < node.cnt; ++pos)
					if (node.key[pos] == leaf.data[0].first) break;
				for (int i = pos; i < node.cnt - 1; ++i)
					node.key[i] = node.key[i + 1], node.ch[i] = node.ch[i + 1];
				node.cnt--;

				writeFile(&left, left.offset, 1, sizeof(leafNode));
				if (check_node(node) == Success) writeFile(&node, node.offset, 1, sizeof(internalNode));
				else operate_node(node);
				return Success;
			}

			/**
			 * function: check if node needs to operate;
			 * return Fail if needs, return Success otherwise.
			 */
			inline OperationResult check_node(internalNode node) {
				if(node.par == 0) return Success;
				if(node.cnt >= MMIN) return Success;
				return Fail;
			}

			/**
			 * function: update leaf to satisfy bpt's needs.
			 */
			void operate_leaf(leafNode leaf) {
				if(borrow_right(leaf) == Success) return;
				if(borrow_left(leaf) == Success) return;
				if(merge_right(leaf) == Success) return;
				if(merge_left(leaf) == Success) return;
				writeFile(&leaf, leaf.offset, 1, sizeof(leafNode));
			}

			/**
			 * function: update internal nodes to satisfy bpt's needs.
			 */
			void operate_node(internalNode node) {
				if(borrow_right_node(node) == Success) return;
				if(borrow_left_node(node) == Success) return;
				if(merge_right_node(node) == Success) return;
				if(merge_left_node(node) == Success) return;
				// only one father
				internalNode par;
				readFile(&par, node.par, 1, sizeof(internalNode));
				if(par.par == 0) {
					info.root = node.offset;
					node.par = 0;
					writeFile(&info, info_offset, 1, sizeof(basicInfo));
					writeFile(&node, node.offset, 1, sizeof(internalNode));
				} else {
					internalNode ppar;
					readFile(&ppar, par.par, 1, sizeof(internalNode));
					for (int i = 0; i < ppar.cnt; ++i)
						if (ppar.ch[i] == par.offset) {
							ppar.ch[i] = node.offset;
							break;
						}
					node.par = ppar.offset;
					writeFile(&ppar, ppar.offset, 1, sizeof(internalNode));
					writeFile(&node, node.offset, 1, sizeof(internalNode));
				}
			}

			/**
			 * function: borrow from right brother.
			 * return Success if succcess, return Fail if fail.
			 */
			OperationResult borrow_right_node(internalNode node) {
				if(node.par == 0) return Fail;
				internalNode par;
				readFile(&par, node.par, 1, sizeof(internalNode));
				int pos = 0;
				for (; pos < par.cnt; ++pos)
					if (par.ch[pos] == node.offset) break;
				if (pos == par.cnt) throw "??";
				if (pos == par.cnt - 1) return Fail;
				internalNode right;
				readFile(&right, par.ch[pos + 1], 1, sizeof(internalNode));
				if(right.cnt <= MMIN) return Fail;

				node.key[node.cnt] = right.key[0];
				node.ch[node.cnt] = right.ch[0];
				++ node.cnt;
				for (int i = 0; i < right.cnt - 1; ++i) right.key[i] = right.key[i + 1];
				for (int i = 0; i < right.cnt - 1; ++i) right.ch[i] = right.ch[i + 1];
				-- right.cnt;

				par.key[pos + 1] = right.key[0];

				// set parent for son
				if(node.type == 1) {						// leaf
					leafNode son;
					readFile(&son, node.ch[node.cnt - 1], 1, sizeof(leafNode));
					son.par = node.offset;
					writeFile(&son, son.offset, 1, sizeof(leafNode));
				} else {
					internalNode son;
					readFile(&son, node.ch[node.cnt - 1], 1, sizeof(internalNode));
					son.par = node.offset;
					writeFile(&son, son.offset, 1, sizeof(internalNode));
				}

				writeFile(&node, node.offset, 1, sizeof(internalNode));
				writeFile(&right, right.offset, 1, sizeof(internalNode));
				writeFile(&par, par.offset, 1, sizeof(internalNode));
				return Success;
			}

			/**
			 * function: borrow from right brother.
			 * return Success if success, return Fail if fail.
			 */
			OperationResult borrow_left_node(internalNode node) {
				if(node.par == 0) return Fail;
				internalNode par;
				readFile(&par, node.par, 1, sizeof(internalNode));
				int pos = 0;
				for (; pos < par.cnt; ++pos)
					if (par.ch[pos] == node.offset) break;
				if (pos == par.cnt) throw "??";
				if (pos == 0) return Fail;
				internalNode left;
				readFile(&left, par.ch[pos - 1], 1, sizeof(internalNode));
				if(left.cnt <= MMIN) return Fail;

				for (int i = node.cnt - 1; i >= 0; -- i) node.key[i + 1] = node.key[i];
				for (int i = node.cnt - 1; i >= 0; -- i) node.ch[i + 1] = node.ch[i];
				node.key[0] = left.key[left.cnt - 1];
				node.ch[0] = left.ch[left.cnt - 1];
				++ node.cnt;
				-- left.cnt;

				par.key[pos] = node.key[0];

				if(node.type == 1) {						// leaf
					leafNode son;
					readFile(&son, node.ch[0], 1, sizeof(leafNode));
					son.par = node.offset;
					writeFile(&son, son.offset, 1, sizeof(leafNode));
				} else {
					internalNode son;
					readFile(&son, node.ch[0], 1, sizeof(internalNode));
					son.par = node.offset;
					writeFile(&son, son.offset, 1, sizeof(internalNode));
				}

				writeFile(&node, node.offset, 1, sizeof(internalNode));
				writeFile(&left, left.offset, 1, sizeof(internalNode));
				writeFile(&par, par.offset, 1, sizeof(internalNode));
				return Success;
			}

			/**
 			 * function: merge with right brother.
 			 * return Success if success, return Fail if fail.
 			 */
			OperationResult merge_right_node(internalNode node) {
				if (node.par == 0) return Fail;
				internalNode par;
				readFile(&par, node.par, 1, sizeof(internalNode));
				int pos = 0;
				for (; pos < par.cnt; ++pos)
					if (par.ch[pos] == node.offset) break;
				if(pos == par.cnt) throw "???";
				if(pos == par.cnt - 1) return Fail;
				internalNode right;
				readFile(&right, par.ch[pos + 1], 1, sizeof(internalNode));

				for (int i = 0; i < right.cnt; ++ i) {
					node.key[node.cnt] = right.key[i];
					node.ch[node.cnt] = right.ch[i];
					if (node.type == 1) {
						leafNode son;
						readFile(&son, node.ch[node.cnt], 1, sizeof(leafNode));
						son.par = node.offset;
						writeFile(&son, son.offset, 1, sizeof(leafNode));
					} else {
						internalNode son;
						readFile(&son, node.ch[node.cnt], 1, sizeof(internalNode));
						son.par = node.offset;
						writeFile(&son, son.offset, 1, sizeof(internalNode));
					}
					++ node.cnt;
				}

				for (int i = pos + 1; i < par.cnt - 1; ++i)
					par.key[i] = par.key[i+1], par.ch[i] = par.ch[i+1];
				-- par.cnt;
				writeFile(&node, node.offset, 1, sizeof(internalNode));
				if(check_node(par) == Success) writeFile(&par, par.offset, 1, sizeof(internalNode));
				else operate_node(par);
				return Success;
			}
			/**
			 * function: merge with left brother.
			 * return Success if success, return Fail if fail.
			 */
			OperationResult merge_left_node(internalNode node) {
				if (node.par == 0) return Fail;
				internalNode par;
				readFile(&par, node.par, 1, sizeof(internalNode));
				int pos = 0;
				for (; pos < par.cnt; ++pos)
					if (par.ch[pos] == node.offset) break;
				if(pos == par.cnt) throw "???";
				if(pos == 0) return Fail;
				internalNode left;
				readFile(&left, par.ch[pos - 1], 1, sizeof(internalNode));

				for (int i = 0; i < node.cnt; ++ i) {
					left.key[left.cnt] = node.key[i];
					left.ch[left.cnt] = node.ch[i];
					if (left.type == 1) {
						leafNode son;
						readFile(&son, left.ch[left.cnt], 1, sizeof(leafNode));
						son.par = left.offset;
						writeFile(&son, son.offset, 1, sizeof(leafNode));
					} else {
						internalNode son;
						readFile(&son, left.ch[left.cnt], 1, sizeof(internalNode));
						son.par = left.offset;
						writeFile(&son, son.offset, 1, sizeof(internalNode));
					}
					++ left.cnt;
				}

				for (int i = pos; i < par.cnt - 1; ++i)
					par.key[i] = par.key[i+1], par.ch[i] = par.ch[i+1];
				-- par.cnt;
				writeFile(&left, left.offset, 1, sizeof(internalNode));
				if(check_node(par) == Success) writeFile(&par, par.offset, 1, sizeof(internalNode));
				else operate_node(par);
				return Success;
			}


		public:
			class iterator {
					friend class BTree;
				private:
					offset_t offset;        // offset of the leaf node
					int place;							// place of the element in the leaf node
					BTree *from;
				public:
					iterator() {
						from = nullptr;
						place = 0, offset = 0;
					}
					iterator(BTree *_from, offset_t _offset = 0, int _place = 0) {
						from = _from;
						offset = _offset; place = _place;
					}
					iterator(const iterator& other) {
						from = other.from;
						offset = other.offset;
						place = other.place;
					}
					iterator(const const_iterator& other) {
						from = other.from;
						offset = other.offset;
						place = other.place;
					}

					// to get the value type pointed by iterator.
					ValueType getValue() {
						leafNode p;
						from -> readFile(&p, offset, 1, sizeof(leafNode));
						return p.data[place].second;
					}

					OperationResult modify(const ValueType& value) {
						leafNode p;
						from -> readFile(&p, offset, 1, sizeof(leafNode));
						p.data[place].second = value;
						from -> writeFile(&p, offset, 1, sizeof(leafNode));
						return Success;
					}

					// Return a new iterator which points to the n-next elements
					iterator operator++(int) {
						iterator ret = *this;
						// end of bptree
						if(*this == from -> end()) {
							from = nullptr; place = 0; offset = 0;
							return ret;
						}
						leafNode p;
						from -> readFile(&p, offset, 1, sizeof(leafNode));
						if(place == p.cnt - 1) {
							if(p.nxt == 0) ++ place;
							else {
								offset = p.nxt;
								place = 0;
							}
						} else ++ place;
						return ret;
					}
					iterator& operator++() {
						if(*this == from -> end()) {
							from = nullptr; place = 0; offset = 0;
							return *this;
						}
						leafNode p;
						from -> readFile(&p, offset, 1, sizeof(leafNode));
						if(place == p.cnt - 1) {
							if(p.nxt == 0) ++ place;
							else {
								offset = p.nxt;
								place = 0;
							}
						} else ++ place;
						return *this;
					}
					iterator operator--(int) {
						iterator ret = *this;
						if(*this == from -> begin()) {
							from = nullptr; place = 0; offset = 0;
							return ret;
						}
						leafNode p, q;
						from -> readFile(&p, offset, 1, sizeof(leafNode));
						if(place == 0) {
							offset = p.pre;
							from -> readFile(&q, p.pre, 1, sizeof(leafNode));
							place = q.cnt - 1;
						} else -- place;
						return ret;
					}
					iterator& operator--() {
						if(*this == from -> begin()) {
							from = nullptr; place = 0; offset = 0;
							return *this;
						}
						leafNode p, q;
						from -> readFile(&p, offset, 1, sizeof(leafNode));
						if(place == 0) {
							offset = p.pre;
							from -> readFile(&q, p.pre, 1, sizeof(leafNode));
							place = q.cnt - 1;
						} else -- place;
						return *this;
					}
					bool operator==(const iterator& rhs) const {
						return offset == rhs.offset && place == rhs.place && from == rhs.from;
					}
					bool operator==(const const_iterator& rhs) const {
						return offset == rhs.offset && place == rhs.place && from == rhs.from;
					}
					bool operator!=(const iterator& rhs) const {
						return offset != rhs.offset || place != rhs.place || from != rhs.from;
					}
					bool operator!=(const const_iterator& rhs) const {
						return offset != rhs.offset || place != rhs.place || from != rhs.from;
					}
			};

			class const_iterator {
					friend class BTree;
				private:
					offset_t offset;        // offset of the leaf node
					int place;							// place of the element in the leaf node
					const BTree *from;
				public:
					const_iterator() {
						from = nullptr;
						place = 0, offset = 0;
					}
					const_iterator(const BTree *_from, offset_t _offset = 0, int _place = 0) {
						from = _from;
						offset = _offset; place = _place;
					}
					const_iterator(const iterator& other) {
						from = other.from;
						offset = other.offset;
						place = other.place;
					}
					const_iterator(const const_iterator& other) {
						from = other.from;
						offset = other.offset;
						place = other.place;
					}
					// to get the value type pointed by iterator.
					ValueType getValue() {
						leafNode p;
						from -> readFile(&p, offset, 1, sizeof(leafNode));
						return p.data[place].second;
					}
					// Return a new iterator which points to the n-next elements
					const_iterator operator++(int) {
						const_iterator ret = *this;
						// end of bptree
						if(*this == from -> cend()) {
							from = nullptr; place = 0; offset = 0;
							return ret;
						}
						leafNode p;
						from -> readFile(&p, offset, 1, sizeof(leafNode));
						if(place == p.cnt - 1) {
							if(p.nxt == 0) ++ place;
							else {
								offset = p.nxt;
								place = 0;
							}
						} else ++ place;
						return ret;
					}
					const_iterator& operator++() {
						if(*this == from -> cend()) {
							from = nullptr; place = 0; offset = 0;
							return *this;
						}
						leafNode p;
						from -> readFile(&p, offset, 1, sizeof(leafNode));
						if(place == p.cnt - 1) {
							if(p.nxt == 0) ++ place;
							else {
								offset = p.nxt;
								place = 0;
							}
						} else ++ place;
						return *this;
					}
					const_iterator operator--(int) {
						const_iterator ret = *this;
						if(*this == from -> cbegin()) {
							from = nullptr; place = 0; offset = 0;
							return ret;
						}
						leafNode p, q;
						from -> readFile(&p, offset, 1, sizeof(leafNode));
						if(place == 0) {
							offset = p.pre;
							from -> readFile(&q, p.pre, 1, sizeof(leafNode));
							place = q.cnt - 1;
						} else -- place;
						return ret;
					}
					const_iterator& operator--() {
						if(*this == from -> cbegin()) {
							from = nullptr; place = 0; offset = 0;
							return *this;
						}
						leafNode p, q;
						from -> readFile(&p, offset, 1, sizeof(leafNode));
						if(place == 0) {
							offset = p.pre;
							from -> readFile(&q, p.pre, 1, sizeof(leafNode));
							place = q.cnt - 1;
						} else -- place;
						return *this;
					}
					bool operator==(const iterator& rhs) const {
						return offset == rhs.offset && place == rhs.place && from == rhs.from;
					}
					bool operator==(const const_iterator& rhs) const {
						return offset == rhs.offset && place == rhs.place && from == rhs.from;
					}
					bool operator!=(const iterator& rhs) const {
						return offset != rhs.offset || place != rhs.place || from != rhs.from;
					}
					bool operator!=(const const_iterator& rhs) const {
						return offset != rhs.offset || place != rhs.place || from != rhs.from;
					}
			};

			// Default Constructor and Copy Constructor

			BTree() {
				fp_name.setName(ID);
				fp = nullptr;
				openFile();
				if (file_already_exists == 0) build_tree();
			}

			BTree(const BTree& other) {
				fp_name.setName(ID);
				openFile();
				copyFile(fp_name.str, other.fp_name.str);
			}

			BTree& operator=(const BTree& other) {
				fp_name.setName(ID);
				openFile();
				copyFile(fp_name.str, other.fp_name.str);
			}

			~BTree() {
				closeFile();
			}

			/**
			 * Insert: Insert certain Key-Value into the database
			 * Return a pair, the first of the pair is the iterator point to the new
			 * element, the second of the pair is Success if it is successfully inserted
			 */
			pair <iterator, OperationResult> insert(const KeyType& key, const ValueType& value) {
				offset_t leaf_offset = locate_leaf(key, info.root);
				leafNode leaf;
				if(info.size == 0 || leaf_offset == 0) {					// smallest elements
					readFile(&leaf, info.head, 1, sizeof(leafNode));
					pair <iterator, OperationResult> ret = insert_leaf(leaf, key, value);
					if(ret.second == Fail) return ret;
					offset_t offset = leaf.par;
					internalNode node;
					while(offset != 0) {
						readFile(&node, offset, 1, sizeof(internalNode));
						node.key[0] = key;
						writeFile(&node, offset, 1, sizeof(internalNode));
						offset = node.par;
					}
					return ret;
				}
				readFile(&leaf, leaf_offset, 1, sizeof(leafNode));
				pair <iterator, OperationResult> ret = insert_leaf(leaf, key, value);
				return ret;
			}

			/**
			 * Erase: Erase the Key-Value
			 * Return Success if it is successfully erased
			 * Return Fail if the key doesn't exist in the database
			 */
			OperationResult erase(const KeyType& key) {
				offset_t leaf_offset = locate_leaf(key, info.root);
				if(leaf_offset == 0) return Fail;
				leafNode leaf;
				readFile(&leaf, leaf_offset, 1, sizeof(leafNode));
				int pos = 0;
				for (; pos < leaf.cnt; ++pos)
					if (leaf.data[pos].first == key) break;
				if (pos == leaf.cnt) return Fail;          // not found.
				// erase in leaf...
				for (int i = pos + 1; i < leaf.cnt; ++i)
					leaf.data[i - 1].first = leaf.data[i].first, leaf.data[i - 1].second = leaf.data[i].second;
				leaf.cnt --;
				// if is the head of the leaf, then update ancestors.
				offset_t internal_offset = leaf.par;
				internalNode node;
				while(pos == 0) {
					if(internal_offset == 0) break;
					readFile(&node, internal_offset, 1, sizeof(internalNode));
					pos = 0;
					for (; pos < node.cnt; ++pos)
						if (node.key[pos] == key) break;
					node.key[pos] = leaf.data[0].first;
					writeFile(&node, node.offset, 1, sizeof(internalNode));
					internal_offset = node.par;
				}
				info.size --;
				writeFile(&info, info_offset, 1, sizeof(basicInfo));
				if(leaf.cnt < LMIN) {
					operate_leaf(leaf);
					return Success;
				}
				writeFile(&leaf, leaf_offset, 1, sizeof(leafNode));
				return Success;
			}

			// Return a iterator to the beginning
			iterator begin() {
				return iterator(this, info.head, 0);
			}
			const_iterator cbegin() const {
				return const_iterator(this, info.head, 0);
			}
			// Return a iterator to the end(the next element after the last)
			iterator end() {
				leafNode tail;
				readFile(&tail, info.tail, 1, sizeof(leafNode));
				return iterator(this, info.tail, tail.cnt);
			}
			const_iterator cend() const {
				leafNode tail;
				readFile(&tail, info.tail, 1, sizeof(leafNode));
				return const_iterator(this, info.tail, tail.cnt);
			}
			// Check whether this BTree is empty
			bool empty() const {return info.size == 0;}
			// Return the number of <K,V> pairs
			size_t size() const {return info.size;}
			// Clear the BTree
			void clear() {
				fp = fopen(fp_name.str, "w");
				fclose(fp);
				openFile();
				build_tree();
			}
			/**
			 * Returns the number of elements with key
			 *   that compares equivalent to the specified argument,
			 * The default method of check the equivalence is !(a < b || b > a)
			 */
			size_t count(const KeyType& key) const {
				return static_cast <size_t> (find(key) != cend());
			}
			ValueType at(const KeyType& key){
				iterator it = find(key);
				leafNode leaf;
				if(it == end()) {
					throw "not found";
				}
				readFile(&leaf, it.offset, 1, sizeof(leafNode));
				return leaf.data[it.place].second;
			}
			/**
			 * Finds an element with key equivalent to key.
			 * key value of the element to search for.
			 * Iterator to an element with key equivalent to key.
			 *   If no such element is found, past-the-end (see end()) iterator is
			 * returned.`
			 */
			iterator find(const KeyType& key) {
				offset_t leaf_offset = locate_leaf(key, info.root);
				if(leaf_offset == 0) return end();
				leafNode leaf;
				readFile(&leaf, leaf_offset, 1, sizeof(leafNode));
				for (int i = 0; i < leaf.cnt; ++i)
					if (leaf.data[i].first == key) return iterator(this, leaf_offset, i);
				return end();
			}
			const_iterator find(const KeyType& key) const {
				offset_t leaf_offset = locate_leaf(key, info.root);
				if(leaf_offset == 0) return cend();
				leafNode leaf;
				readFile(&leaf, leaf_offset, 1, sizeof(leafNode));
				for (int i = 0; i < leaf.cnt; ++i)
					if (leaf.data[i].first == key) return const_iterator(this, leaf_offset, i);
				return cend();
			}
			/**
			 * this is a simple debug function for B Tree's ID number.
			 * very simple, use it if necessary.
			 */
			void debugID() { std :: cerr << ID << ' ' << fp_name.str <<std :: endl;}
			/**
			 * this is a debug function for traverse.
			 * just call it if necessary.
			 * it will present all the elements in B Tree with its value type.
			 */
			void debug_traverse() {
				basicInfo infoo;
				readFile(&infoo, info_offset, 1, sizeof(basicInfo));
				offset_t cur = infoo.head;
				leafNode leaf;
				std :: cout << info.head << ' ' << info.tail << std :: endl;
				std :: cout << "\nbegin traverse: \n";
				while(1) {
					readFile(&leaf, cur, 1, sizeof(leafNode));
					std :: cout << "leaf size = " << leaf.cnt << std :: endl;
					for (int i=0; i<leaf.cnt; ++i) std :: cout << "(" << leaf.data[i].first << ", " << leaf.data[i].second << ")  ";
					if(cur == infoo.tail) break;
					cur = leaf.nxt;
				}
				std :: cout << '\n';
				std :: cout << "=====================\n";
			}
			/**
			 * this is a debug function for iterator
			 * just call it if necessary
			 * it will present all the elements in B Tree with its value type by iterator operations.
			 */
			void debug_iterator() {
				iterator it;
				it = begin();
				while(it != end()) {
					std :: cout << it.getValue() << ' ';
					++it;
				}
			}
			/**
			 * this is a debug function for const_iterator
			 * just call it if necessary
			 * it will present all the elements in B Tree with its value type by const_iterator operations.
			 */
			void debug_const_iterator() {
				const_iterator it;
				it = cbegin();
				while(it != cend()) {
					std :: cout << it.getValue() << ' ';
					++it;
				}
			}
	};
}  // namespace sjtu
