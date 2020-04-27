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

template <typename T>
vector<size_t> sort_indexes_decending(const vector<T> &v) {

    // initialize original index locations
    vector<size_t> idx(v.size());
    iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    // using std::stable_sort instead of std::sort
    // to avoid unnecessary index re-orderings
    // when v contains elements of equal values
    stable_sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

    return idx;
}

void set_player_parameters(array<player_qlearning*,4> players, double epsilon, double alpha, double gamma){
    for (int i =0;i<4;i++) {
        players[i]->set_parameters(epsilon, alpha, gamma);
    }
}

array<player_qlearning*,4> get_new_players( double epsilon=0.3, double alpha=0.5, double gamma=0.2){
    array<player_qlearning*,4> players;
    for (int i = 0;i<4;i++){
        players[i] = new player_qlearning;
    }
    return players;
};

void train(game &g,array<player_qlearning*,4> players, int iterations=10000){
    g.set_players(players[0],players[1],players[2],players[3]);
    int wins[] = {0, 0, 0, 0};
    for(int i = 0; i < iterations; i++)
    {
        g.reset();
        g.set_first(i%4); //alternate who starts the game
        g.play_game();
        wins[g.get_winner()]++;
    }
    cout<< "Wins percentages "<< (double)wins[0]/iterations<<"\t"<< (double)wins[1]/iterations<<"\t"<< (double)wins[2]/iterations<<"\t"<< (double)wins[3]/iterations<<"\t"<<"%"<<endl;
}
vector<size_t> evaluate(game &g, int iterations = 1000){
    int mostWins[4];
    vector<int> wins = {0, 0, 0, 0};
    for(int i = 0; i < iterations; i++)
    {
        g.reset();
        g.set_first(i%4); //alternate who starts the game
        g.play_game();
        wins[g.get_winner()]++;
    }
    cout<< "Wins percentages "<< (double)wins[0]/iterations<<"\t"<< (double)wins[1]/iterations<<"\t"<< (double)wins[2]/iterations<<"\t"<< (double)wins[3]/iterations<<"\t"<<"%"<<endl;
    return sort_indexes_decending(wins);

}


void print_qvals(std::array<int,4> A){
    for (int v : A)
        cout<<v<<",";
    cout<<endl;
}


int main()
{
    auto players = get_new_players();
    game g;
    train(g,players,50000);
    players[0]->set_parameters(1,0,0);
    players[1]->set_parameters(1,0,0);
    players[2]->set_parameters(0.0,0,0);
    players[3]->set_parameters(0.0,0,0);
    vector<std::size_t> mwp;
    mwp = evaluate(g);
    cout<<mwp[0]<<" "<<mwp[1]<<" "<<mwp[2]<<" "<<mwp[3]<<" "<<endl;

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