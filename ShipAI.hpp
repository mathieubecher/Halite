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
			
			// Si le vaisseau à suffisement de ressources pour entreprendre un retour
            if (ship->halite >= 800 * game->turn_number/constants::MAX_TURNS ||
				// Tant que le vaisseau a encore assez de ressources
				(ships_infos[ship->id].dropoff && ship->halite >= 500 * game->turn_number / constants::MAX_TURNS)) {
				return _return_dropoff();
			}
			// Sinon le vaisseau va récolter de l'halite
			else {
			   ships_infos[ship->id].dropoff = false;
				return _search_halite();
			}


		}


		// Recherche le dropoff / shipyard le plus proche
		Position NearDropOff() {
			// Position du shipyard
			Position nearDropOff = game->players[ship->owner]->shipyard->position;
			int nearDistance = game->game_map->calculate_distance(nearDropOff, ship->position);

			shared_ptr<Player> player = game->players[ship->owner];

			// Position de chaque dropoff
			for (int i = 0; i < dropoffs.size(); ++i) {
				int actualDist = game->game_map->calculate_distance(dropoffs[i], ship->position);
				if (actualDist < nearDistance ) {
					nearDropOff = dropoffs[i];
				}
			}
			return nearDropOff;

		}


		// ACTION DE RETOUR AU DROPOFF
		Command _return_dropoff() {
			shared_ptr<Player> player = game->me;

			// Si le vaisseau décide de rentrer on cherche le dropoff le plus proche (une seul fois)
			if (!ships_infos[ship->id].dropoff) {
				ships_infos[ship->id].position = NearDropOff();
			}
			ships_infos[ship->id].dropoff = true;

			// Si il est plus rentable de rester sur la case
			if (game_map->at(ship)->halite > constants::MAX_HALITE / 10 && !ship->is_full()) return ship->stay_still();

			// Si le dropoff le plus proche est très loin et que il n'y a pas de dropoff ennemi à proximitée
			if (player->dropoffs.size() < 2 && !dropoff && game->players[ship->owner]->halite > constants::DROPOFF_COST && game_map->calculate_distance(ships_infos[ship->id].position, ship->position) > 15
				&& (enemyDropoffInRange(ship->position, 4, player->id, game_map).size() ==  0)) {
				// informe les autres vaisseaux qu'il y'a déja un dropoff créé ce tour ci 
				dropoff = true;

				dropoffs.push_back(ship->position);
				return ship->make_dropoff();
			}
			
			// Regarde si le calcul de direction n'échoue pas, le vaisseau va vers le dropoff. sinon, il va chercher de l'halite en attendant
			Direction d = game_map->naive_navigate(ship, ships_infos[ship->id].position);
			if (d == Direction::STILL) return _search_halite();
			return ship->move(d);
			

		}



			   
		// ACTION DE RECHERCHE D'HALITE
		Command _search_halite() {
			// Recherche parmis les cases voisines si il y'a plus d'halite 
			int maxHalite = 0;
			// Par défaut, la direction est null
			Direction dirMaxHalite = Direction::STILL;
			// Ne reste pas fixe si il y a un ennemi proche
			bool forceMove = false;
			// Parcours en diamant des cases voisines
			for (int x = -1; x <= 1; ++x) {
				for (int y = -(1 - abs(x)); y <= (1 - abs(x)); ++y) {
					// Position de la case adjacente a tester
					Position p = ship->position + Position(x, y);
					Direction d = game_map->naive_navigate(ship, p);
					if (d != Direction::STILL && game_map->at(p)->halite > maxHalite) {
						if (enemyShipsInRange(p, 1, ship->owner, game_map).size() > 0) forceMove = true;
						// Si la case est atteignable et qu'il n'y a pas d'ennemi proche on regarde si elle a plus d'halite que les autres
						else {
							dirMaxHalite = d;
							maxHalite = game_map->at(p)->halite;
						}
					}
					
				}
			}
			// Si la case est vidé et qu'il y'a des cases autour plus rentable le vaisseau se déplace
			if ((maxHalite > 0 && forceMove) || game_map->at(ship)->halite < constants::MAX_HALITE / 10 && (maxHalite > game_map->at(ship)->halite || game_map->at(ship)->has_structure())) {
				return ship->move(dirMaxHalite);

			}
			// Sinon il reste
			return ship->stay_still();

		}
		
		// ACTION DE RECOLTE
		Command _recolt() {

			return ship->stay_still();

		}

	};

}