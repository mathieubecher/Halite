using namespace std;
using namespace hlt;



namespace shipBTree {
	
	enum OrderType {
		DROPOFF, AGGRO
	};

	struct Order {
		OrderType type;
		float weight;
		Position target;
		shared_ptr<Ship> owner;
		Order(OrderType _type, float _weight, Position _target):type(_type),weight(_weight),target(_target){}

		float shipWeight(Ship ship) {

			if (type == DROPOFF) {
				return 1;
			}
			return 0;
		}

	};
	vector<Order> orders;
}
