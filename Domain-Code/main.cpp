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

array<player_qlearning *, 4> get_new_players(double epsilon = 0.1, double alpha = 0.1, double gamma = 0.5) {
    array<player_qlearning *, 4> players;
    for (int i = 0; i < 4; i++) {
        players[i] = new player_qlearning;
        players[i]->set_parameters(epsilon, alpha, gamma);
    }
    return players;
};

void train(game &g, array<player_qlearning *, 4> players, int iterations = 10000) {
    g.set_players(players[0], players[1], players[2], players[3]);
    int wins[] = {0, 0, 0, 0};
    for (int i = 0; i < iterations; i++) {
        g.reset();
        g.set_first(i % 4); //alternate who starts the game
        g.play_game();
        wins[g.get_winner()]++;
    }
}


template<class A>
ofstream &operator<<(ofstream &os, vector<A> &data) {
    int cols = data[0].size();
    int rows = data.size() - 1;// -1 tp avoid newline on last entry
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            os << data[i][j] << " ";
        os << "\n";
    }
    for (int j = 0; j < cols; j++)
        os << data[rows][j] << " ";
}

void train(game &g, array<player_qlearning *, 4> players, int iterations = 10000, string log_path = " ",
           bool append = false) {
    bool log = log_path != " ";
    ofstream log_file;
    if (append) {
        log_file.open(log_path, std::ofstream::out | std::ofstream::app);
        log_file << "\n";
    } else {
        log_file.open(log_path, std::ofstream::out | std::ofstream::trunc);
    }
    g.set_players(players[0], players[1], players[2], players[3]);
    int wins[] = {0, 0, 0, 0};
    vector<array<int, 28>> winLog;
    array<int, 28> data = {0};
    winLog.reserve(iterations);
    int winner = 0;
    for (int i = 0; i < iterations; i++) {
        g.reset();
        g.set_first(i % 4); //alternate who starts the game
        g.play_game();

        winner = g.get_winner();
        if (log) {
            data.fill(0);
            data[winner] = 1;
            for (int p = 0; p < 4; p++) {
                data[p + 4] = players[p]->acc_reward;
                data[p + 8] = players[p]->qTable->size();
                data[p + 12] = g.player_was_sent_home[p];
                data[p + 16] = g.player_sent_someone_home[p];
                data[p + 20] = g.stars_hit[p];
                data[p + 24] = g.globes_hit[p];
            }
            winLog.push_back(data);
        }
    }
    log_file << winLog;
    log_file.close();
    //cout<< "Wins percentages "<< (double)wins[0]/iterations<<"\t"<< (double)wins[1]/iterations<<"\t"<< (double)wins[2]/iterations<<"\t"<< (double)wins[3]/iterations<<"\t"<<"%"<<endl;
}

double evaluate(game &g, player_qlearning *player, int iterations = 1000,double eval_epsilon=0,double eval_alpha=0) {
    double epsilon = player->epsilon;
    double alpha = player->epsilon;
    player->epsilon =eval_epsilon;
    player->alpha = 0.0;
    int mostWins[4];
    vector<int> wins = {0, 0, 0, 0};
    for (int i = 0; i < iterations; i++) {
        g.reset();
        g.set_first(i % 4); //alternate who starts the game
        g.play_game();
        wins[g.get_winner()]++;
    }
    player->epsilon = epsilon;
    player->alpha = alpha;
   // cout << "Wins percentages " << (double) wins[0] / iterations << "\t";
    return (double) wins[0] / iterations;
}


void print_qvals(std::array<int, 4> A) {
    for (int v : A)
        cout << v << ",";
    cout << endl;
}


int main() {

    auto players = get_new_players(0.3, 0.3, 0.3);
    player_random player_r_1;
    player_random player_r_2;
    player_random player_r_3;


    game g;
    game ge0(players[0], &player_r_1, &player_r_2, &player_r_3);
    game ge1(players[1], &player_r_1, &player_r_2, &player_r_3);
    game ge2(players[2], &player_r_1, &player_r_2, &player_r_3);
    game ge3(players[3], &player_r_1, &player_r_2, &player_r_3);


    int epochs = 2000;
    int train_itterations = 10;
    int eval_itterations = 100;
    bool append = false;
    vector<array<double,4>> winPercentData;
    double eval_epsilon =0.0;
    array<double,4> winPercent;
    for (int e = 0; e < epochs; e++) {
        if (e == 1)
            append=true;
        train(g, players, train_itterations, "../data/train.txt", append);
        cout << "Epoch " << e << "\r"<<std::flush;

        winPercent[0] = evaluate(ge0, players[0], eval_itterations,eval_epsilon);
        winPercent[1] = evaluate(ge1, players[1], eval_itterations,eval_epsilon);
        winPercent[2] = evaluate(ge2, players[2], eval_itterations,eval_epsilon);
        winPercent[3] = evaluate(ge3, players[3], eval_itterations,eval_epsilon);
        winPercentData.push_back(winPercent);
    //    cout<<endl;
    }
    ofstream eval_file("../data/eval.txt");
    eval_file<<winPercentData;
    eval_file.close();



//    players[0]->set_parameters(1,0,0);
//    players[1]->set_parameters(1,0,0);
//    players[2]->set_parameters(1,0,0);
//    players[3]->set_parameters(0.0,0.1,0.5);


    // TODO make the player qtable pointers such that they can be swapped easily

//    player_qlearning player_q_0;
//    player_qlearning player_q_1;
//    player_qlearning player_q_2;
//    player_qlearning player_q_3;
//
//    //Play a game of Ludo
//    game gq(&player_q_0, &player_q_1, &player_q_2, &player_q_3);
//
//    int wins[] = {0, 0, 0, 0};
//    auto start = std::chrono::high_resolution_clock::now();
//    int itt  = 10000;
//
//    for (int i = 0;i<itt;i++){
//        gq.reset();
//        gq.set_first(i); //alternate who starts the game
//        gq.play_game();
//        wins[gq.winner]++;
//    }
//    auto finish = std::chrono::high_resolution_clock::now();
//    std::chrono::duration<double> elapsed = finish - start;
//    cout << "q size :"<<" "<<player_q_0.qTable.size()<<" "<<player_q_1.qTable.size()<<" "<<player_q_2.qTable.size()<<" "<<player_q_3.qTable.size()<<endl;
//
//
//    std::cout << "Games pr second(r): " << itt/elapsed.count() << "\n";
//    for(int i = 0; i < 4; i++)
//        cout << "Player " << i << " won " << wins[i] << " games." << endl;
//
//    gq.reset();
//    gq.set_first(1); //alternate who starts the game
//    gq.play_game_with_replay("./replayLudo.txt");


    //train();


    return 0;


}




//int num_threads = 8;
//int rc;
//int i;
//pthread_t threads[num_threads];
//pthread_attr_t attr;
//void *status;
//auto start = std::chrono::high_resolution_clock::now();
//
//// Initialize and set thread joinable
//pthread_attr_init(&attr);
//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
//for( i = 0; i < num_threads; i++ ) {
//cout << "main() : creating thread, " << i << endl;
//rc = pthread_create(&threads[i], &attr, train, (void *)i );
//if (rc) {
//cout << "Error:unable to create thread," << rc << endl;
//exit(-1);
//}
//}
//
//// free attribute and wait for the other threads
//pthread_attr_destroy(&attr);
//for( i = 0; i < num_threads; i++ ) {
//rc = pthread_join(threads[i], &status);
//if (rc) {
//cout << "Error:unable to join," << rc << endl;
//exit(-1);
//}
//cout << "Main: completed thread id :" << i ;
//cout << "  exiting with status :" << status << endl;
//}
//
//cout << "Main: program exiting." << endl;
//auto finish = std::chrono::high_resolution_clock::now();
//std::chrono::duration<double> elapsed = finish - start;
//std::cout << "Games pr second: " << 10000*num_threads/elapsed.count() << " s\n";

//pthread_exit(NULL);