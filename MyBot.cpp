#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"
#include "ShipAI.hpp"
#include "OrderAI.hpp"
#include <random>
#include <ctime>

using namespace std;
using namespace hlt;
using namespace shipBTree;
mt19937 rng;
vector<OrderAI> ordersAI;

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
	ordersAI = vector<OrderAI>();
	ordersAI.push_back(DropOffOrder(&game));
	

    // At this point "game" variable is populated with initial map data.
    // This is a good place to do computationally expensive start-up pre-processing.
    // As soon as you call "ready" function below, the 2 second per turn timer will start.
    game.ready("MathieuAlois");

    log::log("Successfully created bot! My Player ID is " + to_string(game.my_id) + ". Bot rng seed is " + to_string(rng_seed) + ".");

    for (;;) {
        game.update_frame();
        shared_ptr<Player> me = game.me;
        unique_ptr<GameMap>& game_map = game.game_map;
		shipAI.dropoff = false;
        vector<Command> command_queue;
		orders = vector<Order>();
		for (OrderAI& orderAI : ordersAI) {
			orderAI.DefineOrder();
		}


        for (const auto& ship_iterator : me->ships) {
            shared_ptr<Ship> ship = ship_iterator.second;
			command_queue.push_back(shipAI.update(ship));
           
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

