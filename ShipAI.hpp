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

		vector<MapCell*> cellsInRange(Position p, int range, unique_ptr<GameMap>& game_map) {
			vector<MapCell*> retCellsInRange = vector<MapCell*>();
			for (int x = -range; x <= range; ++x) {
				for (int y = -(range - abs(x)); y <= +(range - abs(x)); ++y) {
					if (x == 0 && y == 0) break;
					Position ret;
					ret.x = p.x + x;
					ret.y = p.y + y;
					retCellsInRange.push_back(game_map->at(ret));
				}
			}
			return retCellsInRange;
		}

		vector<shared_ptr<Ship>> shipsInRange(Position p, int range, unique_ptr<GameMap>& game_map) {
			vector<shared_ptr<Ship>> retShipsInRange = vector<shared_ptr<Ship>>();

			for (int x = -range; x <= range; ++x) {
				for (int y = -(range - abs(x)); y <= +(range - abs(x)); ++y) {
					if (x == 0 && y == 0) break;
					Position ret;
					ret.x = p.x + x;
					ret.y = p.y + y;
					if (game_map->at(ret)->is_occupied())
						retShipsInRange.push_back(game_map->at(ret)->ship);
				}
			}

			return retShipsInRange;
		}

		vector<shared_ptr<Ship>> enemyShipsInRange(Position p, int range, PlayerId id, unique_ptr<GameMap>& game_map) {
			vector<shared_ptr<Ship>> retShipsInRange = vector<shared_ptr<Ship>>();

			for (int x = -range; x <= range; ++x) {
				for (int y = -(range - abs(x)); y <= +(range - abs(x)); ++y) {
					if (x == 0 && y == 0) break;
					Position ret;
					ret.x = p.x + x;
					ret.y = p.y + y;
					if (game_map->at(ret)->is_occupied()) {
						shared_ptr<Ship> neighbour = game_map->at(ret)->ship;
						if(neighbour->owner != id)
							retShipsInRange.push_back(neighbour);
					}
				}
			}

			return retShipsInRange;
		}


		void clearNextPos() {
			nextPos = vector<Position>();
		}

		Command update(shared_ptr<Ship> _ship) {
			ship = _ship;

			unique_ptr<GameMap>& game_map = game->game_map;

			vector<shared_ptr<Ship>> enemyInRange = enemyShipsInRange(ship->position, 4, ship->owner, game_map);
			log::log("test neighbour " + enemyInRange.size());

			if (enemyInRange.size() > 0) {
				return _escape();
			}
			else if (ship->halite > 800) {
				return _return_dropoff();
			}
			else if (game_map->at(ship)->halite < constants::MAX_HALITE / 10) {

				return _search_halite();

			}

			return _recolt();
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

		Command _pursue(shared_ptr<Ship> victim) {
			Direction d = game_map->naive_navigate(ship, victim->position);
			if (!isNextPos(ship->position + d)) {
				nextPos.push_back(ship->position + d);
				return ship->move(d);
			}
			return ship->stay_still();
		}

		Command _escape(vector<shared_ptr<Ship>> ships) {
			Position direction(0, 0);
			for (int i = 0; i < ships.size(); ++i) {
				direction += ships[0]->position - ship->position;
			}
			direction = game_map->normalize(direction);
			direction.x = -direction.x; direction.y = -direction.y;

			Direction d = game_map->naive_navigate(ship, direction);
			if (!isNextPos(ship->position + d)) {
				nextPos.push_back(ship->position + d);
				return ship->move(d);
			}
			return ship->stay_still();
		}
	};
}