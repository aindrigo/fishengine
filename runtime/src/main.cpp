#include "fish/game.hpp"
#include "fish/version.hpp"
int main()
{
    fish::Game game("Runtime Test", { 0, 0, 0, fish::Phase::ALPHA });

    game.start();
    game.stop();
}