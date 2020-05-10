using namespace std;
using namespace hlt;


#include "Order.hpp"

namespace shipBTree {

	struct OrderAI {
		Game * game;

		unique_ptr<GameMap>& game_map;

		OrderAI(Game * _game) : game_map(_game->game_map), game(_game) {}
		
		void DefineOrder() {
			log::log("OrderAI");
		}
	};

	struct DropOffOrder : OrderAI{
		DropOffOrder(Game * _game) : OrderAI(_game){}
		void DefineOrder() {
			log::log("DropOffOrder");
		}
	};
}