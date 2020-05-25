//
// Created by jens on 5/21/20.
//

#ifndef DOMAIN_CODE_TRAINING_AND_EVALUATION_H
#define DOMAIN_CODE_TRAINING_AND_EVALUATION_H

#include <iostream>
#include "game.h"
#include "test_game.h"
#include "iplayer.h"
#include "player_qlearning.h"
#include "player_random.h"
#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <pthread.h>


using namespace std;


template<typename T>
vector<size_t> sort_indexes_decending(const vector<T> &v) {

    // initialize original index locations
    vector<size_t> idx(v.size());
    iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    // using std::stable_sort instead of std::sort
    // to avoid unnecessary index re-orderings
    // when v contains elements of equal values
    stable_sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) { return v[i1] > v[i2]; });

    return idx;
}



void set_player_parameters(array<player_qlearning *, 4> players, double epsilon, double alpha, double gamma) {
    for (int i = 0; i < 4; i++) {
        players[i]->set_parameters(epsilon, alpha, gamma);
    }
}

template<typename playerT>
vector<iplayer *> get_player_vector(int nr_players) {
    vector<iplayer *> players;
    for (int i = 0; i < nr_players; i++) {
        players.push_back(new playerT);
    }
    return players;
};
vector<iplayer *> get_player_vector(int nr_players,double epsilon = 0.1, double alpha = 0.1, double gamma = 0.5){
    vector<iplayer * > players = get_player_vector<player_qlearning>(nr_players);
    for (iplayer* player:players){
        dynamic_cast<player_qlearning*>(player)->set_parameters(epsilon,alpha,gamma);
    }
}


template<typename playerT>
array<iplayer *,4> get_player_array() {
    array<iplayer *,4> players;
    for (int i = 0; i < 4; i++) {
        players[i] = (new playerT);
    }
    return players;
};

array<iplayer *,4> get_player_array(double epsilon = 0.1, double alpha = 0.1, double gamma = 0.5){
    array<iplayer *,4> players = get_player_array<player_qlearning>();
    for (iplayer* player:players){
        dynamic_cast<player_qlearning*>(player)->set_parameters(epsilon,alpha,gamma);
    }
    return players;
}


const char delim = ',';

template<class T>
ofstream &operator<<(ofstream &os, vector<T> &data) {
    int elements = data.size();
    for (int i = 0; i < elements-1; i++) // -1 tp avoid newline on last entry
        os << data[i] << delim;
    os << data[elements-1];
}

template<class T>
void write_vector_vector(ofstream &os, vector<vector<T>> &data) {
    int rows = data.size();

    for (int i = 0; i < rows-1; i++){ // -1 tp avoid newline on last entry
        os << data[i];
        os << "\n";
    }
    os << data[rows-1];
}


// ----- training ------
void train(array<iplayer *, 4> players,vector<int> q_player_idx, int iterations = 10000) {
    game g;
    vector<player_qlearning*> qpps;
    for (int i:q_player_idx)
        qpps.push_back(dynamic_cast<player_qlearning*>(players[i]));

    g.set_players(players[0], players[1], players[2], players[3]);
    for (int i = 0; i < iterations; i++) {
        g.reset();
        g.set_first(i % 4); //alternate who starts the game
        g.play_game();
        for (player_qlearning* qpp:qpps)
            qpp->reset();
    }
}

template<class T>
void writeDataToFile(vector<vector<T>> data,int nr_players, int data_field_pr_player, vector<string> data_field_names,int iterations, string log_path, bool append){
    int nr_data_fields = data[0].size();

    if (data_field_names.size() != nr_data_fields/nr_players)
        cout<<"data fields string vector does not have the same size as specified";

    ofstream log_file;
    if (append) {
        log_file.open(log_path, std::ofstream::out | std::ofstream::app);
        log_file << "\n";
    } else {
        log_file.open(log_path, std::ofstream::out | std::ofstream::trunc);
        log_file<<nr_players<<delim<<data_field_pr_player<<delim<<iterations<<delim;
        log_file<<data_field_names;
        log_file<<"\n";
    }

    write_vector_vector(log_file,data);

    log_file.close();
}

void train(array<iplayer*, 4> &players,vector<int> q_player_idx, int iterations, string log_path,
           bool append = false) {
    game g;
    g.set_players(players);

    int data_field_pr_player=7;
    int nr_q_players = q_player_idx.size();
    int nr_data_fields = data_field_pr_player*nr_q_players;

    vector<vector<int>> data_log;
    data_log.resize(iterations);

    vector<int> data;
    data.resize(nr_data_fields);
    int winner = 0; // winner index
    int qpi = 0; // q player index
    player_qlearning* qpp; // q player pointer
    int max_q_was_0=0;
    int max_q_was_not_0=0;

    for (int i = 0; i < iterations; i++) {
        g.reset();
        g.set_first(i % 4); //alternate who starts the game
        g.play_game();
        winner = g.get_winner();

        for (int qp = 0; qp < nr_q_players; qp++) {
            qpi = q_player_idx[qp];
            data[qp] = static_cast<int>(winner==qpi);
            qpp = dynamic_cast<player_qlearning*>(players[qpi]);
            data[qp + 1 * nr_q_players] = qpp->acc_reward;
            data[qp + 2 * nr_q_players] = qpp->qTable->size();
            data[qp + 3 * nr_q_players] = g.player_was_sent_home[qpi];
            data[qp + 4 * nr_q_players] = g.player_sent_someone_home[qpi];
            data[qp + 5 * nr_q_players] = g.stars_hit[qpi];
            data[qp + 6 * nr_q_players] = g.globes_hit[qpi];
            qpp->reset();
        }

        data_log[i]=data;
    }

    vector<string> data_field_names = {"Win","Accumulated reward","Q-table size","Player was sent home","Player sent someone home","# of stars hit pr game","# of globes hit pr game"};
    writeDataToFile(data_log,nr_q_players,data_field_pr_player,data_field_names,iterations,log_path,append);
}


vector<double> evaluate(array<iplayer*,4> &players,vector<int> eval_idxs, int iterations) {


    game g;
    g.set_players(players);
    vector<player_qlearning*> qpps;
    for (int i:eval_idxs)
        qpps.push_back(dynamic_cast<player_qlearning*>(players[i]));

    vector<int> wins = {0, 0, 0, 0};
    for (int i = 0; i < iterations; i++) {

        g.set_first(i%4);
        g.play_game();

        wins[g.get_winner()]++;
        for(player_qlearning* qpp:qpps)
            qpp->reset();

        g.reset();
    }


    int total_wins = 0;
    int nr_eval_players = eval_idxs.size();
    vector<double> player_win_percent;

    for (int i = 0;i<nr_eval_players;i++){
        player_win_percent.push_back(static_cast<double>(static_cast<double>(wins[eval_idxs[i]])/iterations));
        total_wins += wins[eval_idxs[i]];
    }
    player_win_percent.push_back(static_cast<double>(total_wins)/iterations);


    return player_win_percent;
}
vector<double> evaluate(array<iplayer*,4> players,int eval_idx, int iterations ) {
    vector<int> eval_idxs = {eval_idx};
    return evaluate(players,eval_idxs,iterations);
}

vector<double> evaluate(array<iplayer*,4> players,vector<int> eval_idxs, int iterations,double eval_epsilon,double eval_alpha,double eval_gamma) {
    vector<player_qlearning*> qpps;
    vector<array<double,3>> q_params;

    for (int i: eval_idxs){
        player_qlearning* qpp = dynamic_cast<player_qlearning*>(players[i]);
        qpps.push_back(qpp);
        q_params.push_back(array<double,3>{qpp->epsilon,qpp->alpha,qpp->gamma});
        qpp->set_parameters(eval_epsilon,eval_alpha,eval_gamma);
    }

    vector<double> win_percent = evaluate(players,eval_idxs,iterations);

    for (int i = 0;i<qpps.size();i++)
        qpps[i]->set_parameters(q_params[i][0],q_params[i][1],q_params[i][2]);


    return win_percent;
}


void print_qvals(std::array<int, 4> A) {
    for (int v : A)
        cout << v << ",";
    cout << endl;
}



#endif //DOMAIN_CODE_TRAINING_AND_EVALUATION_H
