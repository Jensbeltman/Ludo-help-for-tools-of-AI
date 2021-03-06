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
#define state_variables_per_piece 5
#define number_of_state_variables 7 * 4

typedef float qType;
typedef std::array<int, number_of_state_variables> stateA;
typedef std::array<qType, pieces_per_player> actionA;
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

    std::array<move_evaluation, 4> move_eval; // 4 move evaluation for each piece containing [new_pose,number off colision,who was sent home]

    int piece_to_move = 0;
    int prev_position[4]={-1,-1,-1,-1};



    std::pair<stateA, actionA> newStateActionPair;

    std::array<bool, pieces_per_player> piece_is_valid_option = {false, false, false, false};
    std::vector<int> player_options;
    std::vector<int> max_qs;

    int enemyCol, start, globe, star, goal_area, enemys_behind, stuck;

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

        for(int i =0;i<4;i++)
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

        player_options.clear();
        for (int i = 0; i < 4; i++) {

            piece_is_valid_option[i] = is_valid_move(i);
            if (piece_is_valid_option[i]==true)
                player_options.push_back(i);
        }

        if (player_options.empty()) //no legal moves available
            return -1;


        update_state();
        update_post_round_reward();
        update_qtable();


        if (player_options.size() == 1) //only one legal move
            piece_to_move = player_options[0];
        else if (randDouble() < epsilon) {
            piece_to_move = player_options[distributions[player_options.size() - 1](generator)];
        } else {
            qtable::iterator qarray = qTable->find(state);
            actionA::iterator max = std::max_element(qarray->second.begin(),qarray->second.end());

            max_qs.clear();
            for (int i = 0; i < 4; i++)
                if (qarray->second[i]==(*max) && piece_is_valid_option[i]==true)
                    max_qs.push_back(i);

            int max_size = max_qs.size();
            if (max_size>1){
                piece_to_move = max_qs[distributions[max_size - 1](generator)];
            }else if (max_size==1){
                piece_to_move = max_qs[0];
            }else{
                piece_to_move = player_options[distributions[player_options.size() - 1](generator)];
            }


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


    void update_pre_round_reward() {
        reward = 0;
        int st = square_type(position[piece_to_move]);
        int st_eval = square_type(move_eval[piece_to_move].square_type);
        int positionDiff = move_eval[piece_to_move].position - position[piece_to_move];

        if (positionDiff < 0) {
            reward -= 0.5;
        } else if (st == HOME) {
            reward += 0.5;
        } else {
            if (move_eval[piece_to_move].collisions == 1) // if we didnt get sent home and reward is increased and collision happened
                reward += 0.5;

            if (st_eval == STAR) // if we land on star
                reward += 0.25;

            if (st_eval == GLOBE) // if we land on globe
                reward += 0.1;
            else if (st == GLOBE) // if not not land on globa and was on one before
                reward -= 0.1;


            if (st != GOAL_AREA)
                if (st_eval == GOAL_AREA)
                    reward += 0.5;
            if (st_eval == GOAL_AREA)
                reward += 0.5;
            if (move_eval[piece_to_move].position == 99)
                if (st != GOAL_AREA)
                    reward += 0.1;
                else
                    reward += 0.5;
        }
    }

    void update_post_round_reward() {
        int players_sent_home=0;
        for (int p = 0;p<4;p++)
            if (prev_position[p] > -1 && prev_position[p]<51 && position[p] == -1)
                players_sent_home++;

        reward-=0.5*players_sent_home;
    }

    void update_qtable() {

        auto state_to_update_itt = qTable->find(state_prev);
        auto new_state_itt = qTable->find(state);
        int newStateQ = 0;

        int stateMaxQ;

        if (new_state_itt == qTable->end()) {
            actionA newActionsQs = {0, 0, 0, 0};
            newStateActionPair.first = state;
            qTable->insert(newStateActionPair);
            stateMaxQ = 0;
        } else
            stateMaxQ = std::max(*new_state_itt->second.begin(), *new_state_itt->second.begin());

        qType qval = qTable->operator[](state_prev)[piece_to_move];
        qType qval_prev = qTable->operator[](state_prev)[piece_to_move];
        qTable->operator[](state_prev)[piece_to_move] +=
                alpha * (reward + gamma * stateMaxQ - qTable->operator[](state_prev)[piece_to_move]);
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

        int offset = 0;
        int st_eval = HOME;
        int st_curr = square_type(position[piece_to_move]);
        for (int p = 0; p < pieces_per_player; p++) {
            offset = state_variables_per_piece * p;

            if (piece_is_valid_option[p]) {
                evaluate_move(p);
                st_eval = move_eval[p].square_type;
                enemyCol = count_opponents(move_eval[p].collisions,2);
                start = int(move_eval[p].position == 0);
                globe = int(st_eval == GLOBE);
                star = int(st_eval == STAR);

                if (st_curr != GOAL_AREA)
                    goal_area = int(st_eval == GOAL_AREA);
                else
                    goal_area = 2; // 2 then indicates that the piece is in the goal area


                if (st_curr == GOAL_AREA)
                    enemys_behind=0;
                else
                    enemys_behind = get_danger_level(p, 6);
                stuck = 0;
            } else {
                enemyCol = 0;
                start = 0;
                globe = 0;
                star = 0;
                goal_area = 0;
                enemys_behind = 0;
                stuck = 1;
            }

            state[offset + 0] = enemyCol;
            state[offset + 1] = start;
            state[offset + 2] = globe;
            state[offset + 3] = star;
            state[offset + 4] = goal_area;
            state[offset + 5] = enemys_behind;
            state[offset + 6] = stuck;

//            if (stuck==1)
//                state[offset + 6] = 0;
//            else
//                state[offset + 6] = dice;
        }
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
                return;
            } else if (piece_square > 56) {
                move_eval[piece].position = piece_square - piece_square % 56;
                return;
            } else if (piece_square > 50) {
                move_eval[piece].position = piece_square;
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
                    return;
                } else {
                    move_eval[piece].position = piece_square;
                    return;
                }
            }
        }
    }


    int count_opponents(int square, int max = 4) {
        int count = 0;
        for (int i = 4; i < piece_count; i++) // loop all pieces
            if (position[i] == square)    // on the square
                count++; // TODO optimize by stopping for loop when the player in collision is identified ?
                if (count == max)
                    return count;
        return count;
    }


};

#endif // PLAYER_QLEARNING_H
