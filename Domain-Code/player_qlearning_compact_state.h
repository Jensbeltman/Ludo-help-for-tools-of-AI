#ifndef PLAYER_QLEARNING_H
#define PLAYER_QLEARNING_H

#include "random"
#include "iplayer.h"
#include <string>
#include <unordered_map>
#include <iterator>
#include <boost/functional/hash.hpp>
#include <limits>
#define HOME -1
#define START  0
#define NEUTRAL 1
#define GLOBE 2
#define STAR 3
#define GOAL_AREA 4
#define GOAL  5

#define player_count 4
#define pieces_per_player 4
#define piece_count 4 * 4
#define number_of_state_variables 8

typedef float qType;
typedef std::array<int, number_of_state_variables> stateA;
typedef std::array<qType, number_of_state_variables> actionA;
typedef std::unordered_map<stateA, actionA, boost::hash<stateA>> qtable;

struct move_evaluation {
    int position = 0;
    int collisions = 0;
    int square_type = 0;
};

class player_qlearning : public iplayer {
public:

    const int square_type_lookup[56] = {GLOBE, NEUTRAL, NEUTRAL, NEUTRAL, NEUTRAL, STAR, NEUTRAL, NEUTRAL, GLOBE,
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
    stateA state;
    stateA state_prev;
    qType reward = 0;
    qType acc_reward = 0;

    const int start_move = 0;
    const int simple_move = 1;
    const int star_move = 2;
    const int globe_move = 3;
    const int goalArea_move = 4;
    const int goal_move = 5;
    const int coll_good_move = 6;
    const int coll_bad_move = 7;

    std::array<move_evaluation, 4> move_eval; // 4 move evaluation for each piece containing [new_pose,number off colision,who was sent home]



    std::array<bool, pieces_per_player> piece_is_valid_option = {false, false, false, false};
    int nr_options = 0;
    std::vector<int> player_options;
    std::vector<int> move_options;
    std::vector<int> max_q_player_options;
    std::vector<int> max_q_move_options;
    bool stuck = false;

    int piece_to_move = 0;
    int move = 0;
    int prev_position[4]={-1,-1,-1,-1};
    int curr_position[4]={-1,-1,-1,-1};
    double rndn;


    std::random_device rd;
    std::mt19937 generator;
    int generator_max;
    std::vector<std::uniform_int_distribution<int>> distributions;



public:
    player_qlearning() {
        // TODO Consider adding something likeqTable->reserve()
        position=(new int[16]);
        qTable = new qtable;
        generator = std::mt19937(rd());
        generator_max = (int) generator.max();

        for(int i =0;i<number_of_state_variables;i++)
            distributions.push_back(std::uniform_int_distribution<int>(0, i));



        update_state();

        for (move_evaluation mv:move_eval) {
            mv.position = 0;
        }
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
    int make_decision() //Selects legal move at random
    {
        curr_position[0]= position[0];
        curr_position[1]= position[1];
        curr_position[2]= position[2];
        curr_position[3]= position[3];
        player_options.clear();
        move_options.clear();
        for (int i = 0; i < 4; i++) {
            piece_is_valid_option[i] = is_valid_move(i);
            if (piece_is_valid_option[i]==true){
                player_options.push_back(i);
                move_options.push_back(get_move_type(i));
            }
        }
        nr_options = player_options.size();

        if (nr_options == 0){ //no legal moves available
            return -1;
        }
//        if (state[coll_bad_move] && move==coll_bad_move && rndn>epsilon) {
//            qtable::iterator iT = qTable->find(state);
//            actionA aa = iT->second;
//            if (aa[coll_bad_move]<0)
//                int r= 5;
//        }
        update_state();
        update_qtable();

        rndn = randDouble();
        if (player_options.size() == 1) //only one legal move
        {
            piece_to_move = player_options[0];
            move = move_options[0];
        }
        else if (rndn<epsilon) {
            int idx = distributions[player_options.size() - 1](generator);
            piece_to_move = player_options[idx];
            move = move_options[idx];
        } else {
            qtable::iterator state_action_it = qTable->find(state);
            stateA t1 = state_action_it->first;
            actionA t2 = state_action_it->second;
            max_q_player_options.clear();
            max_q_move_options.clear();

            qType max = -1000;

            for (int i =0; i < nr_options; i++)
                if (state_action_it->second[move_options[i]] > max)
                    max=state_action_it->second[move_options[i]];

            for (int i =0; i < nr_options; i++)
                if (state_action_it->second[move_options[i]] == max) {
                    max_q_player_options.push_back(player_options[i]);
                    max_q_move_options.push_back(move_options[i]);
                }

            if (max_q_player_options.size() == 0)
                std::cout<<"pls debug me"<<std::endl;

            int nr_max_q_options = max_q_player_options.size();
            if (nr_max_q_options > 1){
                int idx = distributions[nr_max_q_options - 1](generator);
                piece_to_move = max_q_player_options[idx];
                move = max_q_move_options[idx];
            }else if (nr_max_q_options == 1){
                piece_to_move = max_q_player_options[0];
                move = max_q_move_options[0];

            }else{
               std::cout<<"This need debugging"<<std::endl;
            }
//            if (state[start_move]==1 && state[simple_move]==1)
//                std::cout<<"wtf"<<std::endl;
        }

        update_pre_round_reward();
        acc_reward += reward;

        prev_position[0] = position[0];
        prev_position[1] = position[1];
        prev_position[2] = position[2];
        prev_position[3] = position[3];

        return piece_to_move;
    }


    double randDouble() {
        return ((double) generator() / UINT_MAX);
    }

    void update_qtable() {
        auto state_to_update_itt = qTable->find(state_prev);
        auto new_state_itt = qTable->find(state);
        int stateMaxQ;

        if (new_state_itt == qTable->end()) {
            actionA new_action_a={0};
            qTable->insert(std::make_pair(state,new_action_a));
            stateMaxQ = 0;
        } else
            stateMaxQ = std::max(*new_state_itt->second.begin(), *new_state_itt->second.begin());

        qTable->operator[](state_prev)[move] += alpha * (reward + gamma * stateMaxQ - qTable->operator[](state_prev)[move]);
    }

    void update_pre_round_reward() {
        reward = 0;
        int st = square_type(position[piece_to_move]);
        int st_eval = move_eval[piece_to_move].square_type;
//        int positionDiff = move_eval[piece_to_move].position - position[piece_to_move];

//        if (state[start_move]==1)
//            std::cout<<"test here"<<std::endl;

        if (move==coll_bad_move) {
            reward -= 1.0;
        } else if (move == start_move) {
            reward += 0.5;
        } else {
            if (move==coll_good_move) // if we didnt get sent home and reward is increased and collision happened
                reward += 0.7;

            if (move==simple_move) // if we didnt get sent home and reward is increased and collision happened
                reward += 0.05;

            if (st_eval == STAR) // if we land on star
                reward += 0.6;

           if (st_eval == GLOBE) // if we land on globe
                reward += 0.1;

            if (st != GOAL_AREA)
                if (st_eval == GOAL_AREA)
                    reward += 0.8;

            if (move_eval[piece_to_move].position == 99)
                if (st != GOAL_AREA)
                    reward += 1.0;
                else
                    reward += 0.2;
        }
    }

    int get_move_type(int p){
        if (piece_is_valid_option[p]) {
            int st_curr = square_type(position[p]);
            evaluate_move(p);
            int st_eval = move_eval[p].square_type;


            if(move_eval[p].collisions == 0){
                if (st_curr == HOME)
                    return start_move;
                else if (st_eval==NEUTRAL)
                    return simple_move;
                else if (st_eval==GLOBE)
                    return globe_move;
                else if (st_eval==STAR)
                    return star_move;
                else if (st_eval==GOAL_AREA)
                    if (st_curr!=GOAL_AREA)
                        return goalArea_move;
                    else
                        return simple_move;
                else if (st_eval==GOAL)
                    return goal_move;
            }
            else if (move_eval[p].collisions == 1){
                if (st_eval == GLOBE)
                    return coll_bad_move;
                else
                    return coll_good_move;
            }
            else if (move_eval[p].collisions >1)
                return coll_bad_move;
            else
                std::cout<<"No move was found pls debug me"<<std::endl;
        }
    }
    int square_type(int piece_position) {
        if (piece_position == -1)
            return HOME;
        if (piece_position == 99)
            return GOAL;
        return square_type_lookup[piece_position];
    }


    void update_state() {
        state_prev = state;
        state.fill(0);
        for (int i:move_options)
                state[i]=1;
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

            if (piece_square == 56 || piece_square == 50) {
                move_eval[piece].position = 99;
                move_eval[piece].square_type = square_type(move_eval[piece].position);
                return;
            } else if (piece_square > 56) {
                move_eval[piece].position = piece_square - piece_square % 56;
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

                int opp = count_opponents(piece_square);

                move_eval[piece].collisions = opp;

                if (opp == 0) {
                    move_eval[piece].position = piece_square;
                    return;
                } else if (opp > 1 || move_eval[piece].square_type == GLOBE) {
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


    int count_opponents(int square) {
        int count = 0;
        for (int i = 4; i < piece_count; i++) // loop all pieces
            if (position[i] == square)    // on the square
                count++; // TODO optimize by stopping for loop when the player in collision is identified ?
        return count;
    }


};

#endif // PLAYER_QLEARNING_H
