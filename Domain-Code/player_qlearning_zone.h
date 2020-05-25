#ifndef PLAYER_QLEARNING_H
#define PLAYER_QLEARNING_H

#include "random"
#include "iplayer.h"
#include <string>
#include <unordered_map>
#include <iterator>
#include <boost/functional/hash.hpp>
#include <limits>
#include <memory>
#include <algorithm>
#include "../Players/move_logic.h"
#define HOME -1
#define NEUTRAL 1
#define GLOBE 2
#define STAR 3
#define GOAL_AREA 4
#define GOAL  5

#define number_of_movable_poses 56
#define player_count 4
#define pieces_per_player 4
#define piece_count 4 * 4
#define number_of_zones 8
#define number_of_moves 8
#define number_of_state_variables number_of_moves*number_of_zones

typedef float qType;
typedef std::array<int, number_of_zones> stateA;
typedef std::array<qType, number_of_state_variables> actionA;
typedef std::unordered_map<stateA, actionA, boost::hash<stateA>> qtable;

struct move_evaluation {
    int position = 0;
    int collisions = 0;
    int enemyInCollision = 0;
    int square_type = 0;
};

class option {
public:
    option(){
        piece = 0;
        move = 0;
        zone = 0;
        action_idx = 0;
    }
    option(int piece, int move, int zone):piece(piece),move(move),zone(zone),action_idx(move+zone*number_of_moves){
    }
    int piece,move,zone,action_idx;
    option& operator=(option &other)
    {
        piece = other.piece;
        move = other.move;
        zone = other.zone;
        action_idx = other.action_idx;
        return *this;
    }
};

class player_qlearning : public iplayer {
public:
    move_logic logic;
    const int square_type_lookup[number_of_movable_poses] = {GLOBE, NEUTRAL, NEUTRAL, NEUTRAL, NEUTRAL, STAR, NEUTRAL, NEUTRAL, GLOBE,
                                        NEUTRAL, NEUTRAL, STAR, NEUTRAL, GLOBE, NEUTRAL, NEUTRAL, NEUTRAL, NEUTRAL,
                                        STAR, NEUTRAL, NEUTRAL, GLOBE, NEUTRAL, NEUTRAL, STAR, NEUTRAL, GLOBE, NEUTRAL,
                                        NEUTRAL, NEUTRAL, NEUTRAL, STAR, NEUTRAL, NEUTRAL, GLOBE, NEUTRAL, NEUTRAL,
                                        STAR, NEUTRAL, GLOBE, NEUTRAL, NEUTRAL, NEUTRAL, NEUTRAL, STAR, NEUTRAL,
                                        NEUTRAL, GLOBE, NEUTRAL, NEUTRAL, STAR, GOAL_AREA, GOAL_AREA, GOAL_AREA,
                                        GOAL_AREA, GOAL_AREA};

    const int star_idxs[8] = {5, 11, 18, 24, 31, 37, 44, 50}; // Used for finding next star

    double epsilon = 0.2;
    double alpha = 0.5;
    double gamma = 0.9;

    qtable *qTable;
    stateA state={0};
    stateA state_prev={0};
    qType reward = 0;
    qType acc_reward = 0;

    double self_prog_post_move_weight = 1.0;
    double self_prog_pre_move_weight = 1.0;
    double enemy_prog_weight = 1.0;

    enum move_types
    {
        start_move = 0,
        simple_move = 1,
        star_move = 2,
        globe_move = 3,
        coll_bad_move = 4,
        coll_good_move_e1 = 5,
        coll_good_move_e2 = 6,
        coll_good_move_e3 = 7
    };

//    const int goalArea_move = 4;
//    const int goal_move = 5;
    std::array<move_evaluation, 4> move_eval; // 4 move evaluation for each piece containing [new_pose,number off colision,who was sent home]

    std::array<bool, pieces_per_player> piece_is_valid_option = {false, false, false, false};
    int nr_options = 0;
    std::vector<option> options;
    std::vector<option> max_q_options;
    const int zoneDiv = (int)ceil(number_of_movable_poses/(double)number_of_zones);
    int zone[number_of_movable_poses];

    option action;
    int stateMaxQ=-1000;

    int prev_position[4]={-1,-1,-1,-1};
    double rndn;


    std::random_device rd;
    std::mt19937 generator;
    int generator_max;
    std::vector<std::uniform_int_distribution<int>> distributions;



public:
    player_qlearning() {
        for(int i =0;i<number_of_movable_poses;i++) // initialize zone convertion array
            zone[i]=i/zoneDiv;

        for(int i =0;i<number_of_state_variables;i++) // initialize distrubtion objects for random choices
            distributions.push_back(std::uniform_int_distribution<int>(0, i));

        position=(new int[16]{0});
        qTable = new qtable;
        generator = std::mt19937(rd());
        generator_max = (int) generator.max();

        update_state();

        for (move_evaluation mv:move_eval) {
            mv.position = 0;
            mv.collisions = 0;
            mv.enemyInCollision = 0;
            mv.square_type = 0;
        }

        actionA new_action_a = {0};
        qTable->insert(std::make_pair(state, new_action_a));
    }

    void reset() {
        acc_reward = 0;
        reward=0;
        for (int i = 0; i < 16; i++) {
            position[i] = -1;
        }
        for (int i = 0;i<4;i++)
            prev_position[i] = -1;
    };

    void set_parameters(double epsilon_v, double alpha_v, double gamma_v) {
        epsilon = epsilon_v;
        alpha = alpha_v;
        gamma = gamma_v;
    };

private:
    int get_piece_zone(int piece){
        int p =position[piece];
        if (p == -1)
            return 0;
        if((p == 51 && piece>3) ) // if enemy is behind start pose
            return 0;
        if ((p > 50 && piece>3) || p ==99)// if enemy we dont have relative zone after 50 or if they are at goal(99)
            return zone[number_of_movable_poses-1];

        return zone[position[piece]];
    }

    void update_state() {
        state_prev = state;
        state.fill(0);
        for (int i=4;i<piece_count;i++){
            int z = get_piece_zone(i);
            state[z]=1;
        }
    }

    int get_move_type(int p){
        if (piece_is_valid_option[p]) {
            int st_curr = square_type(position[p]);
            evaluate_move(p);
            int st_eval = move_eval[p].square_type;

            if (st_curr == HOME)// Note that even if players are sent home it return a home move
                return start_move;

            if(move_eval[p].collisions == 0){
                if (st_eval==NEUTRAL)
                    return simple_move;
                else if (st_eval==GLOBE)
                    return globe_move;
                else if (st_eval==STAR)
                    return star_move;
                else if (st_eval==GOAL_AREA)
                    return simple_move;
                else if (st_eval==GOAL)
                    return simple_move;
                else
                    std::cout<<"shit"<<std::endl;
            }
            else if (move_eval[p].collisions == 1){
                if (st_eval == GLOBE)
                    return coll_bad_move;
                else
                    if (move_eval[p].enemyInCollision==1)
                        return coll_good_move_e1;
                    if (move_eval[p].enemyInCollision==2)
                        return coll_good_move_e2;
                    if (move_eval[p].enemyInCollision==3)
                        return coll_good_move_e3;
                    else
                        std::cout<<"shit"<<std::endl;
            }
            else if (move_eval[p].collisions >1)
                return coll_bad_move;
            else
                std::cout<<"No move was found pls debug me"<<std::endl;
        }
    }

    int make_decision() //Selects legal move at random
    {
        update_post_round_reward();

        options.clear();
        for (int i = 0; i < 4; i++) {
            piece_is_valid_option[i] = is_valid_move(i);
            if (piece_is_valid_option[i]==true){
                options.push_back(option(i,get_move_type(i),get_piece_zone(i)));
            }
        }

        nr_options = options.size();

        if (nr_options == 0){ //no legal moves available
            return -1;
        }

        update_state();
        update_qtable();

        rndn = randDouble();
        if (nr_options == 1) //only one legal move
        {
            action = options[0];
        }
        else if (rndn<epsilon) {

            int idx = distributions[nr_options - 1](generator);
            action = options[idx];
        } else {
            // find actions for a given state
            qtable::iterator state_action_it = qTable->find(state);
//            stateA t1 = state_action_it->first;
//            actionA t2 = state_action_it->second;

            // ---- Find action with max Q value ----
            max_q_options.clear();
            qType max = -1000;
            for (int i =0; i < nr_options; i++)
                if (state_action_it->second[options[i].action_idx] > max)
                    max=state_action_it->second[options[i].action_idx];

            for (int i =0; i < nr_options; i++)
                if (state_action_it->second[options[i].action_idx] == max) {
                    max_q_options.push_back(options[i]);
                }

            // Chose action, radnom if multiple possible
            if (max_q_options.size() == 0)
                std::cout<<"pls debug me"<<std::endl;

            int nr_max_q_options = max_q_options.size();
            if (nr_max_q_options > 1){
                int idx = distributions[nr_max_q_options - 1](generator);
                action = max_q_options[idx];
            }else if (nr_max_q_options == 1){
                action = max_q_options[0];
            }else{
               std::cout<<"This need debugging"<<std::endl;
            }
        }

        update_pre_round_reward();
        acc_reward += reward;

        for(int i =0;i<4;i++)
            prev_position[i]=position[i];

        return action.piece;
    }


    double randDouble() {
        return ((double) generator() / UINT_MAX);
    }

    void update_qtable(){
        auto state_to_update_itt = qTable->find(state_prev);
        if (state_to_update_itt==qTable->end()) {
            actionA new_action_a = {0};
            qTable->insert(std::make_pair(state_prev, new_action_a));
            std::cout<<"somethings wierd"<<std::endl;
            state_to_update_itt = qTable->find(state_prev);
        }

        auto new_state_itt = qTable->find(state);

        if (new_state_itt == qTable->end()) {
            actionA new_action_a={0};
            qTable->insert(std::make_pair(state,new_action_a));
            stateMaxQ = 0;
        } else{
            stateMaxQ = new_state_itt->second[options[0].action_idx];
            for(int i = 1;i<nr_options;i++){
                if (new_state_itt->second[options[i].action_idx]>stateMaxQ){
                    stateMaxQ = new_state_itt->second[options[i].action_idx];
                }
              //  stateMaxQ = std::max(*new_state_itt->second.begin(), *new_state_itt->second.begin());

            }
        }
        if (action.move == coll_bad_move)
            stateMaxQ=0;

        state_to_update_itt->second[action.action_idx] += alpha * (reward + gamma * stateMaxQ - state_to_update_itt->second[action.action_idx]);

    }


    void update_post_round_reward(){
        for (int i = 0;i<4;i++){
            reward += (position[i]-prev_position[i])*self_prog_post_move_weight;
        }
    }
    void update_pre_round_reward() {
        reward = 0;

        int moveDiff = (move_eval[action.piece].position - position[action.piece]);
        if (moveDiff<0)
            reward+= moveDiff* self_prog_pre_move_weight;
        if (moveDiff>0)
            reward+=move_eval[action.piece].position* self_prog_pre_move_weight;

        if (action.move==start_move)
            reward+=100;

        switch (action.move) {
            case coll_good_move_e1:
                reward+= ( damage_done(1,move_eval[action.piece].position) + 21 ) * enemy_prog_weight;
            case coll_good_move_e2:
                reward+= ( damage_done(2,move_eval[action.piece].position) + 21 ) * enemy_prog_weight;
            case coll_good_move_e3:
                reward+= ( damage_done(3,move_eval[action.piece].position) + 21 ) * enemy_prog_weight;
        }
    }


    int square_type(int piece_position) {
        if (piece_position == -1)
            return HOME;
        if (piece_position == 99)
            return GOAL;
        return square_type_lookup[piece_position];
    }

    int get_danger_level(int piece, int look_back = 6){
        int enemy_count = 0;
        int diff;
        for (int i = 4;i<16;i++){
            diff = piece-position[i];
            if (-look_back<=diff && diff<0)
                enemy_count++;
        }
        if (enemy_count>1)
            return 2;
        else
            return enemy_count;
    }

    int next_star(int piece_square) {
        for (int s: star_idxs)
            if (s > piece_square)
                return s;
    }

    void evaluate_move(int piece) {
        int piece_square = position[piece];
        move_eval[piece].position = 0;
        if (piece_square == -1) {
            move_eval[piece].position = 0;
            return;
        } else {
            piece_square += dice;

            if (piece_square == number_of_movable_poses || piece_square == 50) {
                move_eval[piece].position = 99;
                move_eval[piece].square_type = square_type(move_eval[piece].position);
                return;
            } else if (piece_square > number_of_movable_poses) {
                move_eval[piece].position = piece_square - piece_square % number_of_movable_poses;
                move_eval[piece].square_type = square_type(move_eval[piece].position);
                return;
            } else if (piece_square > 50) {
                move_eval[piece].position = piece_square;
                move_eval[piece].square_type = square_type(move_eval[piece].position);
                return;
            } else {
                move_eval[piece].square_type = square_type(piece_square);

                if (move_eval[piece].square_type == STAR) {
                    piece_square = next_star(piece_square);
                }

                move_eval[piece].collisions=0;
                move_eval[piece].enemyInCollision=0;

                for (int i = 4; i < piece_count; i++) // loop all pieces
                    if (position[i] == piece_square){    // on the square
                        move_eval[piece].collisions++;
                        move_eval[piece].enemyInCollision=i/4;
                    }

                if ( move_eval[piece].collisions == 0) {
                    move_eval[piece].position = piece_square;
                    return;
                } else if ( move_eval[piece].collisions > 1 || move_eval[piece].square_type == GLOBE) {
                    move_eval[piece].position = -1;
                    move_eval[piece].square_type = square_type(move_eval[piece].position);

                    return;
                } else {
                    move_eval[piece].position = piece_square;
                    return;
                }
            }
        }
    }

    int damage_done(int enemy,int target){
        int damage = 0;
        for(int i = 4*enemy; i < (4*enemy+4); i++)
        {
            if (position[i]==target) {
                damage += target - enemy * 13;
                if (damage < 0)
                    damage += 52;
            }
        }
        return damage;
    }

};

#endif // PLAYER_QLEARNING_H
