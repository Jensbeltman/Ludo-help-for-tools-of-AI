#ifndef PLAYER_RANDOM_H
#define PLAYER_RANDOM_H

#include "random"
#include "iplayer.h"

class player_random : public iplayer
{
private:
    std::mt19937 generator;
    std::uniform_int_distribution<int> distribution;

public:
    player_random()
    {
        std::random_device rd;
        generator = std::mt19937(rd());
    }

private:
    void reset(){
        for(int i =16;i<16;i++){
            position[i]=-1;
        }
    }

    int  make_decision() //Selects legal move at random
    {
        std::vector<int> player_options;
        for(int i = 0; i < 4; i++)
            if(is_valid_move(i)) //implemented in iplayer.h
                player_options.push_back(i);

        if(player_options.size() == 0) //no legal moves available
            return -1;

        if(player_options.size() == 1) //only one legal move
            return player_options[0];

        //Select randomly between multiple legal moves
        distribution = std::uniform_int_distribution<int>(0, player_options.size() - 1);
        return player_options[distribution(generator)];
    }
};

#endif // PLAYER_RANDOM_H
