using namespace std;
using namespace hlt;
#include <map>

namespace shipBTree{
	// Informations des vaisseaux
	struct ShipInfo {
		// Position du dropoff à rejoindre
		Position position;
		// Retourne a un dropoff
		bool dropoff;
		ShipInfo() : dropoff(false), position() {  }
	};

;	// Comportement des vaisseaux
	struct ShipAI {
		// Liste des vaisseaux
		map<int, ShipInfo> ships_infos;
		// Jeu en cours
		Game * game;
		unique_ptr<GameMap>& game_map;

		// Vaisseau en cours de traitement
		shared_ptr<Ship> ship;
		// Liste des dropoffs/shipyard
		vector<Position> dropoffs;
		bool dropoff;


		// Constructeur
		ShipAI(Game * _game) : game_map(_game->game_map),game(_game),dropoffs(){}

		// Retourne le nombre de vaisseaux ennemies dans un rayon donné
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

		// Retourne le nombre de DropOff ennemies dans un rayon donné
		vector<Position> enemyDropoffInRange(Position p, int range, PlayerId id, unique_ptr<GameMap>& game_map) {
			vector<Position> retDropOffInRange = vector<Position>();

			for (int x = -range; x <= range; ++x) {
				for (int y = -(range - abs(x)); y <= +(range - abs(x)); ++y) {
					if (x == 0 && y == 0) break;
					Position ret;
					ret.x = p.x + x;
					ret.y = p.y + y;
					if (game_map->at(ret)->has_structure()) {
						Position neighbour = game_map->at(ret)->structure->position;
						if (game_map->at(ret)->structure->owner != id)
							retDropOffInRange.push_back(neighbour);
					}
				}
			}

			return retDropOffInRange;
		}

		// Comportement des vaisseaux
		Command update(shared_ptr<Ship> _ship) {

			ship = _ship;

			unique_ptr<GameMap>& game_map = game->game_map;
			
			// Si le vaisseau à suffisement de ressources	
            if (ship->halite >= 800 * game->turn_number/constants::MAX_TURNS ||
				(ships_infos[ship->id].dropoff && ship->halite >= 500 * game->turn_number / constants::MAX_TURNS)) {
				return _return_dropoff();
			}
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
				if (game->game_map->calculate_distance(dropoffs[i],ship->position) < game->game_map->calculate_distance(nearDropOff, ship->position)) {
					nearDropOff = dropoffs[i];

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

			
			if (player->dropoffs.size() < 2 && !dropoff && game->players[ship->owner]->halite > constants::DROPOFF_COST && game_map->calculate_distance(ships_infos[ship->id].position, ship->position) > 15
				&& (enemyDropoffInRange(ship->position, 4, player->id, game_map).size() ==  0)) {
				dropoff = true;

				dropoffs.push_back(ship->position);
				return ship->make_dropoff();
			}
			
			

			Direction d = game_map->naive_navigate(ship, ships_infos[ship->id].position);
			if (d == Direction::STILL) return _search_halite();
			return ship->move(d);
			

		}



			   

		Command _search_halite() {


			int maxHalite = 0;

			Direction dirMaxHalite = Direction::STILL;
			bool forceMove = false;
			for (int x = -1; x <= 1; ++x) {
				for (int y = -(1 - abs(x)); y <= (1 - abs(x)); ++y) {
					Position p = ship->position + Position(x, y);
					Direction d = game_map->naive_navigate(ship, p);
					if (d != Direction::STILL && game_map->at(p)->halite > maxHalite) {
						if (enemyShipsInRange(p, 1, ship->owner, game_map).size() > 0) forceMove = true;
						else {
							dirMaxHalite = d;
							maxHalite = game_map->at(p)->halite;
						}
					}
					
				}
			}
			if ((maxHalite > 0 && forceMove) || game_map->at(ship)->halite < constants::MAX_HALITE / 10 && (maxHalite > game_map->at(ship)->halite || game_map->at(ship)->has_structure())) {
				return ship->move(dirMaxHalite);

			}

			return ship->stay_still();

		}
		

		Command _recolt() {

			return ship->stay_still();

		}

	};

}