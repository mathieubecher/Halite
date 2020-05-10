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

Position BestDropOff(Game * game) {
	const int width = game->game_map->width;
	const int height = game->game_map->height;
	int radius = 7;

	vector<Position> dropoffs;
	dropoffs = vector<Position>();
	for (const auto& player : game->players) {
		dropoffs.push_back(player->shipyard->position);
		for (const auto& dropoff : player->dropoffs) {
			dropoffs.push_back(dropoff.second->position);
		}
	}
	
	vector<vector<int>> cases;
	cases = vector<vector<int>>();

	for (int x = 0; x < width; ++x) {
		cases.push_back(vector<int>());
		for (int y = 0; y < height; ++y) {
			bool near = false;
			Position actual = Position(x, y);
			for (const auto& dropoff : dropoffs) near |= game->game_map->calculate_distance(actual, dropoff) < radius;

			cases[x].push_back((near) ? 0 : game->game_map->at(actual)->halite);
		}
	}
	
	Position best = Position(0, 0);
	
	int bestValue = 0;

	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			if (cases[x][y] > 0) {
				int value = 0;
				for (int X = -radius; X <= radius; ++X) {
					for (int Y = -(radius - abs(X)); Y <= (radius - abs(X)); ++Y) {
						value += cases[(x + X + width) % width][(y + Y + height) % height];
					}
				}

				if (value > bestValue) {
					best = Position(x, y);
					bestValue = value;
				}
			}
		}
	}
	log::log(to_string(bestValue));
	return best;
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
	ShipAI shipAI(&game);
	

    // At this point "game" variable is populated with initial map data.
    // This is a good place to do computationally expensive start-up pre-processing.
    // As soon as you call "ready" function below, the 2 second per turn timer will start.
    game.ready("MathieuAlois");

    log::log("Successfully created bot! My Player ID is " + to_string(game.my_id) + ". Bot rng seed is " + to_string(rng_seed) + ".");
	bool test = true;
    for (;;) {
        game.update_frame();

        shared_ptr<Player> me = game.me;
        unique_ptr<GameMap>& game_map = game.game_map;
		shipAI.dropoff = false;
        vector<Command> command_queue;


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
