#ifndef PLAYER_QLEARNING_H
#define PLAYER_QLEARNING_H

#include "random"
#include "iplayer.h"
#include <string>
#include <unordered_map>
#include <iterator>
#include <boost/functional/hash.hpp>

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
#define number_of_state_variables 5 * 4

typedef std::array<int, number_of_state_variables> stateA;
typedef std::array<double, pieces_per_player> actionA;
typedef std::unordered_map<stateA, actionA,boost::hash<stateA>> qtable;

struct move_evaluation
{
    int position=0;
    int collisions=0;
    int square_type=0;
};
class player_qlearning : public iplayer {
public:

    const int square_type_lookup[56] = {GLOBE,NEUTRAL,NEUTRAL,NEUTRAL,NEUTRAL,STAR,NEUTRAL,NEUTRAL,GLOBE,NEUTRAL,NEUTRAL,STAR,NEUTRAL,GLOBE,NEUTRAL,NEUTRAL,NEUTRAL,NEUTRAL,STAR,NEUTRAL,NEUTRAL,GLOBE,NEUTRAL,NEUTRAL,STAR,NEUTRAL,GLOBE,NEUTRAL,NEUTRAL,NEUTRAL,NEUTRAL,STAR,NEUTRAL,NEUTRAL,GLOBE,NEUTRAL,NEUTRAL,STAR,NEUTRAL,GLOBE,NEUTRAL,NEUTRAL,NEUTRAL,NEUTRAL,STAR,NEUTRAL,NEUTRAL,GLOBE,NEUTRAL,NEUTRAL,STAR,GOAL_AREA,GOAL_AREA,GOAL_AREA,GOAL_AREA,GOAL_AREA};
    const int globes_idxs[5] = {0,8,21,34,47};
    const int safe_globes_idxs[5] = {0,8,21,34,47};
    const int star_idxs[8] = {5,11,18,24,31,37,44,50};

    double epsilon = 0.2;
    double alpha = 0.5;
    double gamma = 0.9;

    stateA state;
    stateA state_prev;
    int reward = 0;
    int action = 0;
    std::array<move_evaluation,4> move_eval; // 4 move evaluation for each piece containing [new_pose,number off colision,who was sent home]
    qtable qTable;

    std::pair<stateA,actionA> newStateActionPair;

    std::array<bool,pieces_per_player> piece_is_valid_option = {false,false,false,false};
    int enemyCol,start,globe,star,goal_area,stuck;

    std::random_device rd;
    std::mt19937 generator;
    int generator_max;
    std::uniform_int_distribution<int> distribution;



public:
    player_qlearning() {
        // TODO Consider adding something likeqTable.reserve()
        generator = std::mt19937(rd());
        generator_max = (int)generator.max();
        update_state(piece_is_valid_option);
        for (move_evaluation mv:move_eval){
            mv.position = 0;
        }
    }

    void set_parameters(double epsilon_v, double alpha_v, double gamma_v){
        epsilon = epsilon_v;
        alpha = alpha_v;
        gamma = gamma_v;
    };

private:
    int make_decision() //Selects legal move at random
    {

        std::vector<int> options;
        for (int i = 0; i < 4; i++)
        {

            piece_is_valid_option[i] = is_valid_move(i);
            if (piece_is_valid_option[i])
                options.push_back(i);
        }

        if (options.size() == 0) //no legal moves available
            return -1;

//        std::cout<<"test 1"<<std::endl;

        update_state(piece_is_valid_option);
        update_post_round_reward();
        update_qtable();


        if (options.size() == 1) //only one legal move
            action = options[0];
        else if (randDouble() < epsilon)
        {
            distribution = std::uniform_int_distribution<int>(0, options.size() - 1);
            action = options[distribution(generator)];
        }
        else
        {
            //std::cout<<"Non random aciton"<<std::endl;
            qtable::iterator qarray = qTable.find(state);
            int isend = qarray == qTable.end();
            int max = qarray->second[0];
            for (int i = 0;i<4;i++ )
            {
                if (piece_is_valid_option[i]){
                    if (qarray->second[i] >= max) // todo implement random choise when multiple max values
                    {
                        max = qarray->second[i];
                        action = i;
                    }
                }
            }

        }

        update_pre_round_reward();

        return action;
    }

    double randDouble(){
        return ((double)generator() / UINT_MAX) ;
    }
    void update_pre_round_reward()
    {
        reward = 0;
        int st = square_type(position[action]);
        int st_eval = square_type(move_eval[action].square_type);
        int positionDiff = move_eval[action].position - position[action];

        if(positionDiff<0){
            reward -= 12;
        }
        if (st == HOME){
            reward+=12;
        }

        if (positionDiff > 0) // if we didnt get sent home and reward is increased and collision happened
            reward += move_eval[action].collisions*12;


        if ( st_eval ==  GLOBE) // if we land on globe
            reward += 8;
        else if( st ==  GLOBE ) // if not not land on globa and was on one before
            reward -= 12;

        if (st != GOAL_AREA)
            if (st_eval == GOAL_AREA )
                reward += 12;
            if (st != GOAL_AREA && move_eval[action].position == 99)
                reward+=24;
    }

    void update_post_round_reward(){
        ;
    }

    void update_qtable(){

        auto state_to_update_itt = qTable.find(state_prev);
        auto new_state_itt = qTable.find(state);
        int newStateQ = 0;

        int stateMaxQ;

        if (new_state_itt == qTable.end())
        {
            actionA newActionsQs = {0,0,0,0};
            newStateActionPair.first = state;
            qTable.insert(newStateActionPair);
            stateMaxQ = 0;
        }
        else
            stateMaxQ = std::max(*new_state_itt->second.begin(), *new_state_itt->second.begin());

        int qval = qTable[state_prev][action];
        int qval_prev = qTable[state_prev][action];
        qTable[state_prev][action] += alpha * (reward + gamma * stateMaxQ - qTable[state_prev][action] );
    }

    int square_type(int piece_position){
        if( piece_position == -1)
            return HOME;
        if (piece_position == 99)
            return GOAL;

        return square_type_lookup[piece_position];
    }

    void update_state(std::array<bool,pieces_per_player> piece_is_valid_option) {
        state_prev = state;

        int offset = 0;
        int st = HOME;
        for (int p = 0; p < pieces_per_player; p++) {
            offset = state_variables_per_piece*p;

            if (piece_is_valid_option[p]) {
                evaluate_move(p);
                st = move_eval[p].square_type;
                enemyCol = count_opponents(move_eval[p].collisions);
                start = int(move_eval[p].position==0);
                globe = int(st==GLOBE);
                star = int(st==STAR);
                goal_area = int(st==GOAL_AREA);
                stuck = 0;
            }
            else
                {
                enemyCol = 0;
                start = 0;
                globe = 0;
                star = 0;
                goal_area = 0;
                stuck = 1;
            }

            state[offset+0] = enemyCol;
            state[offset+1] = start;
            state[offset+2] = globe;
            state[offset+3] = star;
            state[offset+4] = goal_area;
            state[offset+5] = stuck;
        }
    }


    int next_star(int piece_square){
        for (int s: star_idxs)
            if (s > piece_square)
                return s;
    }

    void evaluate_move(int piece){
        int piece_square = position[piece];
        move_eval[piece].position = 0;
        if(piece_square == -1){
            move_eval[piece].position = 0;
            return;
        }
        else
        {
            piece_square += dice;

            if(piece_square == 56 || piece_square == 50)  {
                move_eval[piece].position = 99;
                return;
            }
            else if (piece_square > 56)
            {
                move_eval[piece].position = piece_square - piece_square%56;
                return;
            }
            else if (piece_square > 50)
            {
                move_eval[piece].position = piece_square;
                return;
            }
            else
            {
                move_eval[piece].square_type = square_type(piece_square);

                if (move_eval[piece].square_type == STAR)
                {
                    piece_square = next_star(piece_square);
                }

                int opp = count_opponents(piece_square);
                move_eval[piece].collisions= opp;
                if(opp == 0)
                {
                    move_eval[piece].position = piece_square;
                    return;
                }
                else if(opp > 1 || move_eval[piece].square_type == GLOBE)
                {
                    move_eval[piece].position = -1;
                    return;
                }
                else
                {
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
