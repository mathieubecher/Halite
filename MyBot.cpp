#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"
#include "ShipAI.hpp"
#include <random>
#include <ctime>

using namespace std;
using namespace hlt;
using namespace shipBTree;
mt19937 rng;


int main(int argc, char* argv[]) {
    unsigned int rng_seed;
    if (argc > 1) {
        rng_seed = static_cast<unsigned int>(stoul(argv[1]));
    } else {
        rng_seed = static_cast<unsigned int>(time(nullptr));
    }
    rng = mt19937(rng_seed);

    Game game;
	ShipAI shipAI(&game);
	
    game.ready("MathieuAlois");

    log::log("Successfully created bot! My Player ID is " + to_string(game.my_id) + ". Bot rng seed is " + to_string(rng_seed) + ".");

    for (;;) {
        game.update_frame();

        shared_ptr<Player> me = game.me;
        unique_ptr<GameMap>& game_map = game.game_map;
		shipAI.dropoff = false;
        vector<Command> command_queue;

		//pour tout les ships, ont va leurs appeler leur Update qui determinera pour chacun d'entre eux l'action suivante.
        for (const auto& ship_iterator : me->ships) {
            shared_ptr<Ship> ship = ship_iterator.second;
			command_queue.push_back(shipAI.update(ship));
           
        }
		//On construit une tortue si on a la quantit�e d'halite n�cessaire et que le shipyard n'est pas occup�.
        if (
            game.turn_number <= 200 &&
            me->halite >= constants::SHIP_COST && //co�ts
            !game_map->at(me->shipyard)->is_occupied() && //occupation
			!shipAI.dropoff) //si on cr�e un dropoff ce tour-ci, on ne construit pas de vaisseau
        {
            command_queue.push_back(me->shipyard->spawn());
        }

        if (!game.end_turn(command_queue)) {
            break;
        }
    }

    return 0;
}
