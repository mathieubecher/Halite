using namespace std;
using namespace hlt;

namespace shipBTree{
	struct ShipAI {

		Game * game;
		shared_ptr<Ship> ship;
		unique_ptr<GameMap>& game_map;
		bool dropoff = false;

		ShipAI(Game * _game) : game_map(_game->game_map),game(_game){}

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

		bool isInDanger(vector<shared_ptr<Ship>> enemyInRange) {
			bool danger = false;
			for (int i = 0; i < enemyInRange.size(); ++i) {
				if (enemyInRange[i]->halite > ship->halite) danger |= true;
			}
			return danger;
		}
		shared_ptr<Ship> nearest(vector<shared_ptr<Ship>> enemyInRange) {
			shared_ptr<Ship> victim = enemyInRange[0];
			for (int i = 0; i < enemyInRange.size(); ++i) {
				if (game_map->calculate_distance(enemyInRange[i]->position,ship->position) < game_map->calculate_distance(victim->position, ship->position)) victim = enemyInRange[i];
			}
			return victim;
		}

		Command update(shared_ptr<Ship> _ship) {
			ship = _ship;

			unique_ptr<GameMap>& game_map = game->game_map;

			vector<shared_ptr<Ship>> enemyInRange = enemyShipsInRange(ship->position, 2, ship->owner, game_map);

			 if (ship->halite > 800) {
				return _return_dropoff();
			}
			else if (game_map->at(ship)->halite < constants::MAX_HALITE / 10) {
				return _search_halite();
			}

			return _recolt();
		}


		Command _return_dropoff() {
			Position * nearDropOff = &game->players[ship->owner]->shipyard->position;
			shared_ptr<Player> player = game->players[ship->owner];

			for (int i = 0; i < player->dropoffs.size(); ++i) {
				if (game_map->calculate_distance(player->dropoffs[i]->position, ship->position) < game_map->calculate_distance(*nearDropOff, ship->position)) {
					nearDropOff = &player->dropoffs[i]->position;
				}
			}
			/*
			if (player->dropoffs.size() < 2 && !dropoff && game->players[ship->owner]->halite > constants::DROPOFF_COST + 1000 && game_map->calculate_distance(nearDropOff, ship->position) > 10) {
				dropoff = true;
				return ship->make_dropoff();
			
			}
			*/
			Direction d = game_map->naive_navigate(ship, *nearDropOff);
			if (d == Direction::STILL) return _search_halite();
			return ship->move(d);
			
		}

			   
		Command _search_halite() {
			
			int maxHalite = 0;
			
			Direction dirMaxHalite = Direction::STILL;
			for (int x = -1; x <= 1; ++x) {
				for (int y = -(1 - abs(x)); y <= (1 - abs(x)); ++y) {
					Position p = ship->position + Position(x, y);
					Direction d = game_map->naive_navigate(ship, p);
					if (d != Direction::STILL && game_map->at(p)->halite > maxHalite) {
						dirMaxHalite = d;
						maxHalite = game_map->at(p)->halite;
					}
				}
			}
			if (maxHalite > game_map->at(ship)->halite || game_map->at(ship)->has_structure()) {
				return ship->move(dirMaxHalite);
			}
			return ship->stay_still();
		}

		
		Command _recolt() {
			return ship->stay_still();
		}

		Command _pursue(shared_ptr<Ship> victim) {
			Direction d = game_map->naive_navigate(ship, victim->position);
			return ship->move(d);
		}

		Command _escape(vector<shared_ptr<Ship>> ships) {
			Position direction(0, 0);
			log::log("begin escape");
			string debug = "";
			for (int i = 0; i < ships.size(); ++i) {
				debug += "["+to_string((ships[i]->position - ship->position).x) + " " + to_string((ships[i]->position - ship->position).y)+"] ";
				direction = direction + (ships[i]->position - ship->position);
			}
			log::log(debug);
			
			if (direction.x > direction.y) direction= Position(max(-1, min(1, direction.x)), 0);
			else direction = Position(0, max(-1,min(1,direction.y)));

			Direction d = invert_direction(game_map->naive_navigate(ship, ship->position + direction));
			if (!game_map->at(ship->position + d)->is_occupied()) {
				return ship->move(d);
			}
			
			return ship->stay_still();
		}
	};
}