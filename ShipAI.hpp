using namespace std;
using namespace hlt;
#include <map>

namespace shipBTree{
	struct ShipInfo {
		Position position;
		bool dropoff;
		bool create;
		ShipInfo() : dropoff(false), create(false), position() {  }
	};
	struct DropOffInfo {
		Position position;
		shared_ptr<Ship> creator;
		bool create;
		int dist(Position ship, unique_ptr<GameMap>& map) {
			if (create) return map->calculate_distance(ship, position);
			else return  map->calculate_distance(ship, position) + map->calculate_distance(creator->position, position);
		}
	};
	struct ShipAI {

		map<int, ShipInfo> ships_infos;
		Game * game;

		shared_ptr<Ship> ship;

		unique_ptr<GameMap>& game_map;

		vector<DropOffInfo> dropoffs;
		bool dropoff;



		ShipAI(Game * _game) : game_map(_game->game_map),game(_game),dropoffs(){}


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



		//pursue the closest target

		shared_ptr<Ship> shouldPursue(shared_ptr<Ship> _ship, vector<shared_ptr<Ship>> enemyInRange, unique_ptr<GameMap>& game_map) { 

			if (enemyInRange.empty()) return nullptr;
			shared_ptr<Ship> target = enemyInRange[0];

			bool hunt = false;

			int treshold = _ship->halite + _ship->halite / 2;

			for (int i = 0; i < enemyInRange.size(); ++i) {

				if (enemyInRange[i]->halite <= treshold && game_map->calculate_distance(ship->position, enemyInRange[i]->position) <= game_map->calculate_distance(ship->position, target->position)) {

					target = enemyInRange[i];

					hunt = true;

				}

			}

			if (hunt) return target;

			return nullptr;



		}



		bool shouldFlee(shared_ptr<Ship> _ship, vector<shared_ptr<Ship>> enemyInRange, unique_ptr<GameMap>& game_map) {

			for (int i = 0; i < enemyInRange.size(); ++i) {

				if (enemyInRange[i]->halite <= _ship->halite) {

				    return true;

				}

			}

			return false;

		}



		Command update(shared_ptr<Ship> _ship) {

			ship = _ship;

			log::log("return dropoff " + to_string(ship->id) + " : " + to_string(ships_infos[ship->id].dropoff));
			unique_ptr<GameMap>& game_map = game->game_map;
			
			vector<shared_ptr<Ship>> enemyInRange = enemyShipsInRange(ship->position, 2, ship->owner, game_map);

			
            
           if (ship->halite >= 800 || ((ships_infos[ship->id].dropoff && ship->halite >= 500) || (game->turn_number > 380 && ship->halite >= 100))) {
				return _return_dropoff();
			}/*
			else if (shouldFlee(ship, enemyInRange, game_map)) {
				return _escape(enemyInRange);
            }
            else if (game->players[ship->owner]->halite > 1000 && shouldPursue(ship, enemyInRange,game_map) != nullptr) {
				return _pursue(shouldPursue(ship, enemyInRange,game_map));
            }*/
			else {
			   ships_infos[ship->id].dropoff = false;
				return _search_halite();
			}

			return _recolt();

		}



		Position NearDropOff() {


			Position nearDropOff = game->players[ship->owner]->shipyard->position;

			shared_ptr<Player> player = game->players[ship->owner];



			for (int i = 0; i < dropoffs.size(); ++i) {
				if (dropoffs[i].dist(ship->position, game->game_map) < dropoffs[i].dist(nearDropOff, game->game_map)) {
					nearDropOff = dropoffs[i].position;

				}

				
			}

			return nearDropOff;

		}



		Command _return_dropoff() {
			if (!ships_infos[ship->id].dropoff) {
				ships_infos[ship->id].position = NearDropOff();
			}

			ships_infos[ship->id].dropoff = true;

			if (game_map->at(ship)->halite > constants::MAX_HALITE / 10 && !ship->is_full()) return ship->stay_still();

			
			

			shared_ptr<Player> player = game->players[ship->owner];

			/*
			if (player->dropoffs.size() < 2 && !dropoff && game->players[ship->owner]->halite > constants::DROPOFF_COST && game_map->calculate_distance(ships_infos[ship->id].position, ship->position) > 15) {

				dropoff = true;

				dropoffs.push_back(ship->position);
				return ship->make_dropoff();
			}
			*/
			

			Direction d = game_map->naive_navigate(ship, ships_infos[ship->id].position);
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
					if (d != Direction::STILL && enemyShipsInRange(p,1, ship->owner, game_map).size() == 0 && game_map->at(p)->halite > maxHalite) {
						dirMaxHalite = d;
						maxHalite = game_map->at(p)->halite;
					}
				}
			}
			if (game_map->at(ship)->halite < constants::MAX_HALITE / 10 && (maxHalite > game_map->at(ship)->halite  || game_map->at(ship)->has_structure())) {
				return ship->move(dirMaxHalite);

			}

			return ship->stay_still();

		}


		
		

		Command _recolt() {

			return ship->stay_still();

		}

		/*

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

		*/


	};

}