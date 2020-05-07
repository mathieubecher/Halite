using namespace std;
using namespace hlt;

namespace shipBTree{
	struct ShipAI {

		vector<Position> nextPos;
		Game * game;
		shared_ptr<Ship> ship;
		unique_ptr<GameMap>& game_map;
	
		ShipAI(Game * _game) : game_map(_game->game_map),game(_game){}

		bool isNextPos(Position p) {
			for (int i = 0; i < nextPos.size(); ++i) {
				if (p == nextPos[i]) return true;
			}
			return false;
		}

		void clearNextPos() {
			nextPos = vector<Position>();
		}

		Command update(shared_ptr<Ship> _ship) {
			ship = _ship;
			return ship->stay_still();
		}


		Command _return_dropoff() {
			Position nearDropOff = game->players[ship->owner]->shipyard->position;
			shared_ptr<Player> player = game->players[ship->owner];

			for (int i = 0; i < player->dropoffs.size(); ++i) {
				if (game_map->calculate_distance(player->dropoffs[i]->position, ship->position) < game_map->calculate_distance(nearDropOff, ship->position)) {
					nearDropOff = player->dropoffs[i]->position;
				}
			}
			Direction d = game_map->naive_navigate(ship, nearDropOff);
			if (!isNextPos(ship->position + d)) {
				nextPos.push_back(ship->position + d);
				return ship->move(d);
			}
			return ship->stay_still();
		}

			   
		Command _search_halite() {
			int idMaxHalite = 0;
			int maxHalite = 0;
			for (int i = 0; i < 4; ++i) {
				Position p = ship->position + ALL_CARDINALS[i];
				if (game_map->at(p)->is_empty() && !isNextPos(p))
				{
					if (game_map->at(p)->halite > maxHalite) {
						idMaxHalite = i;
						maxHalite = game_map->at(p)->halite;
					}
				}
			}
			if (maxHalite > game_map->at(ship)->halite || game_map->at(ship)->has_structure()) {
				nextPos.push_back(ship->position + ALL_CARDINALS[idMaxHalite]);
				return ship->move(ALL_CARDINALS[idMaxHalite]);
			}
			return ship->stay_still();
		}

		
		Command _recolt() {
			return ship->stay_still();
		}

	};
}