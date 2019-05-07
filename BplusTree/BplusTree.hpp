# include "utility.hpp"
# include "exception.hpp"
# include <functional>
# include <cstddef>

namespace sjtu {

	template <typename KeyType, typename ValueType, typename Compare = std :: less<KeyType> >
	class BTree {
		private:
			enum NodeType {Internal, Leaf};
			static const int NodeSize = 7;
			static const int MininumKey = NodeSize - 1;
			static const int MaxinumKey = NodeSize * 2 - 1;
			static const int MininumSize = MininumKey + 1;
			static const int MaxinumSize = MaxinumKey + 1;

			class BptNode {
				protected:
					NodeType _type;
					int _keyNum;
					KeyType _keyValue[MaxinumKey];
				public:
					BptNode() {}
					virtual ~BptNode() {}
					NodeType getType() const {return _type;}
					void setType(NodeType __type) {_type = __type;}
					int getKeyNum() const {return _keyNum;}
					void setKeyNum(int __keyNum) {_keyNum = __keyNum;}
					KeyType getKeyValue(int i) const {return _keyValue[i];}
					void setKeyValue(int i, KeyType key) {_keyValue[i] = key;}
			};

			class BptLeafNode {};


			class



		public:
	};

}
