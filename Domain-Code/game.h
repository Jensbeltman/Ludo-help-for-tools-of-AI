#ifndef GAME_H
#define GAME_H

#include "random"
#include <iostream>

#include "dice.h"
#include "iplayer.h"
#include "positions_and_dice.h"
#include <array>

class game
{
    friend class test_game; //to allow direct access in the unit tests

public:
    static const int player_count = 4;
    static const int pieces_per_player = 4;
    static const int piece_count = player_count * pieces_per_player;

    int winner;
    int color;                      //the current player to move
    iplayer* players[player_count]; //the 4 player agents
    int position[piece_count];      //the current board position (absolute position)
    dice game_dice;

    std::array<int,4> player_was_sent_home={0};
    std::array<int,4> player_sent_someone_home={0};
    std::array<int,4> stars_hit={0};
    std::array<int,4> globes_hit={0};


    positions_and_dice rel_pos_and_dice; //relative position and dice result to be send to the player agents

    std::mt19937 generator;
    std::uniform_int_distribution<int> distribution;

public:
    game();
    game(iplayer* p0, iplayer* p1, iplayer* p2, iplayer* p3);
    void set_players(iplayer* p0, iplayer* p1, iplayer* p2, iplayer* p3);
    void set_players(std::array<iplayer*,4> &new_players);
    void reset();
    void set_first(int first); //the index of the player to start the game [0 to 3]
    int  get_winner();
    void play_game();
    void play_game_with_replay(std::string file_path);


private: //private helper functions
    bool has_won();
    void next_turn();
    void next_turn_replay(std::array<int,19> &current_global_game_state );
    void update_dice();
    void update_relative_position();
    int  absolute_to_relative(int square);
    int  absolute_to_relative_replay(int square, int playerColor);
    void move_piece(int relative_piece);
    bool is_valid_move(int relative_piece);
    void trusted_move_piece(int relative_piece);
    void enforce_valid_move();
    int  abs_piece_index(int relative_piece_index);
    void move_start(int abs_piece);
    void send_them_home(int square);
    int  is_star(int square);
    bool is_globe(int square);
    int  count_opponents(int square);
};

#endif // GAME_H
