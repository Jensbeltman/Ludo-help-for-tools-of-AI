#include <iostream>
#include "game.h"
#include "test_game.h"
#include "iplayer.h"
#include "player_qlearning.h"
#include "player_random.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <pthread.h>


using namespace std;

void train() {
    player_qlearning player_q_0;
    player_qlearning player_q_1;
    player_qlearning player_q_2;
    player_qlearning player_q_3;
    player_random player_r_0;
    player_random player_r_1;
    player_random player_r_2;
    player_random player_r_3;

    //Play a game of Ludo
    game gq(&player_q_0, &player_r_1, &player_r_2, &player_r_3);

    player_q_0.epsilon = 0.5;
    int wins[] = {0, 0, 0, 0};
    auto start = std::chrono::high_resolution_clock::now();
    int itt  = 100000;
    for(int i = 0; i < itt; i++)
    {
        gq.reset();
        gq.set_first(i%4); //alternate who starts the game
        gq.play_game();
        wins[gq.get_winner()]++;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "q size : " << player_q_0.qTable.size() << endl;

    std::cout << "Games pr second(r): " << itt/elapsed.count() << "\n";
    for(int i = 0; i < 4; i++)
        cout << "Player " << i << " won " << wins[i] << " games." << endl;

    for (int i = 0;i<4;i++)
        wins[i] = 0;
    start = std::chrono::high_resolution_clock::now();
    player_q_0.epsilon = 0.01;

    for(int i = 0; i < itt; i++)
    {
        gq.reset();
        gq.set_first(i%4); //alternate who starts the game
        gq.play_game();
        wins[gq.get_winner()]++;

    }
    cout << "q size : " << player_q_0.qTable.size() << endl;
    finish = std::chrono::high_resolution_clock::now();
    elapsed = finish - start;
    std::cout << "Games pr second(q): " << itt/elapsed.count() << "\n";
    for(int i = 0; i < 4; i++)
        cout << "Player " << i << " won " << wins[i] << " games." << endl;




}

void print_qvals(std::array<int,4> A){
    for (int v : A)
        cout<<v<<",";
    cout<<endl;
}

//void qTableToFile(qtable qTabel,string file_path){
//    ofstream file;
//    file.open(file_path);
//    for (auto state_action_pair :qTabel){
//        file<<state_action_pair.second<<endl;
//    }
//
//
//}


int main()
{
    player_qlearning player_q_0;
    player_qlearning player_q_1;
    player_qlearning player_q_2;
    player_qlearning player_q_3;

    //Play a game of Ludo
    game gq(&player_q_0, &player_q_1, &player_q_2, &player_q_3);

    int wins[] = {0, 0, 0, 0};
    auto start = std::chrono::high_resolution_clock::now();
    int itt  = 10000;

    for (int i = 0;i<itt;i++){
        gq.reset();
        gq.set_first(i); //alternate who starts the game
        gq.play_game();
        wins[gq.winner]++;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "q size :"<<" "<<player_q_0.qTable.size()<<" "<<player_q_1.qTable.size()<<" "<<player_q_2.qTable.size()<<" "<<player_q_3.qTable.size()<<endl;


    std::cout << "Games pr second(r): " << itt/elapsed.count() << "\n";
    for(int i = 0; i < 4; i++)
        cout << "Player " << i << " won " << wins[i] << " games." << endl;

    gq.reset();
    gq.set_first(1); //alternate who starts the game
    gq.play_game_with_replay("./replayLudo.txt");


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