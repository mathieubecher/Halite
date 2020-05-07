#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"

#include <random>
#include <ctime>

using namespace std;
using namespace hlt;

mt19937 rng;

static Position operator+(const Position& position, const Direction& direction) {
	Position out = Position(position.x, position.y);
	out.x += (direction == Direction::WEST) ? -1 :( (direction == Direction::EAST) ? 1 : 0);
	out.y += (direction == Direction::NORTH) ? -1 :( (direction == Direction::SOUTH) ? 1 : 0);
	
	return out;
}
vector<Position> nextPos;
bool isNextPos(Position p) {
	for (int i = 0; i < nextPos.size(); ++i) {
		if (p == nextPos[i]) return true;
	}
	return false;
}

Command updateShip(shared_ptr<Ship> ship, Game * game) {

	unique_ptr<GameMap>& game_map = game->game_map;
	if (ship->halite > 500) {
		Direction d = game_map->naive_navigate(ship, game->players[ship->owner]->shipyard->position);
		if (!isNextPos(ship->position + d)) {
			nextPos.push_back(ship->position + d);
			return ship->move(d);
		}
		
	}
	else if (game_map->at(ship)->halite < constants::MAX_HALITE / 10) {

		// Choix de la case vide la plus charg� en Halite
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
		
	}

	return ship->stay_still();
}

int main(int argc, char* argv[]) {
    unsigned int rng_seed;
    if (argc > 1) {
        rng_seed = static_cast<unsigned int>(stoul(argv[1]));
    } else {
        rng_seed = static_cast<unsigned int>(time(nullptr));
    }
    rng = mt19937(rng_seed);

    Game game;
    // At this point "game" variable is populated with initial map data.
    // This is a good place to do computationally expensive start-up pre-processing.
    // As soon as you call "ready" function below, the 2 second per turn timer will start.
    game.ready("MathieuAlois");

    log::log("Successfully created bot! My Player ID is " + to_string(game.my_id) + ". Bot rng seed is " + to_string(rng_seed) + ".");

    for (;;) {
        game.update_frame();
        shared_ptr<Player> me = game.me;
        unique_ptr<GameMap>& game_map = game.game_map;

        vector<Command> command_queue;
		nextPos = vector<Position>();
        for (const auto& ship_iterator : me->ships) {
            shared_ptr<Ship> ship = ship_iterator.second;
			command_queue.push_back(updateShip(ship,&game));
           
        }

        if (
            game.turn_number <= 200 &&
            me->halite >= constants::SHIP_COST &&
            !game_map->at(me->shipyard)->is_occupied())
        {
            command_queue.push_back(me->shipyard->spawn());
        }

        if (!game.end_turn(command_queue)) {
            break;
        }
    }

    return 0;
}

