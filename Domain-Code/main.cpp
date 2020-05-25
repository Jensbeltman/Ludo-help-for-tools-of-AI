#include <iostream>
#include "game.h"
#include "test_game.h"
#include "iplayer.h"
#include "player_qlearning_zone.h"
#include "../Players/player_random.h"
#include "../Players/player_random_safe.h"
#include "../Players/player_fast.h"
#include "../Players/player_aggro_fast.h"
#include "training_and_evaluation.h"
#include <iomanip>
using namespace std;


int main() {
    test_game tg;
    tg.run_all_tests();

    array<iplayer*,4> players_r = get_player_array<player_random>();
    array<iplayer*,4> players_rs = get_player_array<player_random_safe>();
    array<iplayer*,4> players_f = get_player_array<player_fast>();
    array<iplayer*,4> players_af = get_player_array<player_aggro_fast>();
    array<iplayer*,4> players_q = get_player_array( 0.133, 0.37, 0.2);

    game g;

    //  --- Compoitions ---
    array<iplayer*,4> players_eval_r{players_q[0],players_r[1],players_r[2],players_r[3]};

    // qrrr
    array<iplayer*,4> player_train_r{players_q[0],players_r[1],players_r[2],players_r[3]};


    int epochs = 250;
    int train_itterations = 30;
    int eval_itterations = 100;
    bool append = false;
    vector<vector<double>> winPercentData_r;
    double eval_epsilon =0.0;
    double eval_alpha =0.0;
    double eval_gamma =0.0;

    vector<double> winPercent_r;
    for (int e = 0; e < epochs; e++) {
        cout << "Epoch " << e << "\r"<<std::flush;

        append = bool(e != 0);
        train(player_train_r,vector<int>{0}, train_itterations, "../data/train.txt", append);

        winPercent_r = evaluate(players_eval_r, vector<int>{0}, eval_itterations, eval_epsilon,eval_alpha,eval_gamma);
        winPercentData_r.push_back(winPercent_r);
    }


    player_qlearning* qp = dynamic_cast<player_qlearning*>(player_train_r[0]);

    int statev = qp->qTable->begin()->first.size();
    int actions = qp->qTable->begin()->second.size();
    for(auto qpit = qp->qTable->begin();qpit !=qp->qTable->end(); qpit++){
        if (qpit->first[7]==1) {
            for (int j = 0; j < statev; j++)
                cout << setw(6) << setprecision(4) << qpit->first[j] << "\t";
            cout << endl;
            for (int j = 0; j < actions; j++)
                cout << setw(6) << setprecision(4) << qpit->second[j] << "\t";
            cout << endl;
        }
    }

    ofstream eval_file_r("../data/eval.txt");
    write_vector_vector(eval_file_r,winPercentData_r);
    eval_file_r.close();


/*    test_game tg;
    tg.run_all_tests();

    array<iplayer*,4> players_r = get_player_array<player_random>();
    array<iplayer*,4> players_rs = get_player_array<player_random_safe>();
    array<iplayer*,4> players_f = get_player_array<player_fast>();
    array<iplayer*,4> players_af = get_player_array<player_aggro_fast>();
    array<iplayer*,4> players_q = get_player_array( 0.133, 0.37, 0.2);

    game g;

    //  --- Compoitions ---
    array<iplayer*,4> players_eval_r{players_q[0],players_r[1],players_r[2],players_r[3]};
    array<iplayer*,4> players_eval_a{players_q[0],players_af[1],players_af[2],players_af[3]};
    // qrrr
    array<iplayer*,4> player_train_r{players_q[0],players_r[1],players_r[2],players_r[3]};

    // qaaa
    array<iplayer*,4> player_train_a{players_q[0],players_r[1],players_r[2],players_r[3]};

    // qqqq
    array<iplayer*,4> player_train_q{players_q[0],players_r[1],players_r[2],players_r[3]};

    // qarr
    array<iplayer*,4> player_train_ar{players_q[0],players_r[1],players_r[2],players_r[3]};

    int epochs = 1000;
    int train_itterations = 30;
    int eval_itterations = 100;
    bool append = false;
    vector<vector<double>> winPercentData_r;
    vector<vector<double>> winPercentData_a;
    double eval_epsilon =0.0;
    double eval_alpha =0.0;
    double eval_gamma =0.0;

    vector<double> winPercent_r;
    vector<double> winPercent_a;
    for (int e = 0; e < epochs; e++) {
        cout << "Epoch " << e << "\r"<<std::flush;

        append = bool(e != 0);
        train(player_train_q,vector<int>{0}, train_itterations, "../data_composition_test/train_q.txt", append);

        winPercent_r = evaluate(players_eval_r, vector<int>{0}, eval_itterations, eval_epsilon,eval_alpha,eval_gamma);
        winPercentData_r.push_back(winPercent_r);

        winPercent_a = evaluate(players_eval_a, vector<int>{0}, eval_itterations, eval_epsilon,eval_alpha,eval_gamma);
        winPercentData_a.push_back(winPercent_a);
    }

    ofstream eval_file_r("../data_composition_test/eval_r_q.txt");
    write_vector_vector(eval_file_r,winPercentData_r);
    eval_file_r.close();

    ofstream eval_file_a("../data_composition_test/eval_a_q.txt");
    write_vector_vector(eval_file_a,winPercentData_a);
    eval_file_a.close();*/


/*
    test_game tg;
    tg.run_all_tests();

    int epochs = 5;
    int train_itterations = 5000;
    int eval_itterations = 1000;

    double eval_epsilon =0.0;
    double eval_alpha =0.0;
    double eval_gamma =0.0;



    for (double e = 0.1;e<0.99;e+=0.1) {
        for (double a = 0.1; a <0.99; a += 0.1) {
            for (double g = 0.1; g < 0.99; g += 0.1) {
                vector<double> winPercent;
                vector<vector<double>> winPercentData;
                double wp = 0;
                bool append = false;

                array<iplayer*,4> players_r = get_player_array<player_random>();
                array<iplayer*,4> players_af = get_player_array<player_aggro_fast>();
                array<iplayer*,4> players_q = get_player_array( e, a, g);

                array<iplayer*,4> player_train{players_q[0],players_r[1],players_r[2],players_r[3]};
                array<iplayer*,4> players_eval{players_q[0],players_r[1],players_r[2],players_r[3]};
                string parameter_string = "_epsilon_" + to_string(e) +"_alpha_" + to_string(a) + "_gamma_" + to_string(g);
                string train_save_path = "../data_hyper_parameter_test/train" + parameter_string + ".txt";
                string eval_save_path = "../data_hyper_parameter_test/eval" + parameter_string + ".txt";
                for (int ep = 0; ep < epochs; ep++) {
                    append = bool(ep != 0);

                    train(player_train,vector<int>{0}, train_itterations, train_save_path, append);

                    winPercent = evaluate(players_eval, vector<int>{0}, eval_itterations, eval_epsilon, eval_alpha, eval_gamma);
                    winPercentData.push_back(winPercent);
                    wp+=winPercent[0];
                }
                cout << parameter_string<<" average win rate is " << wp/epochs<<endl;

                ofstream eval_file(eval_save_path);
                write_vector_vector(eval_file,winPercentData);
                eval_file.close();

            }
        }
    }
*/

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