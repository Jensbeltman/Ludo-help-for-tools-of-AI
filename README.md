# Ludo-help-for-tools-of-AI
The C++ Ludo code on Black Board contains game play bugs and bad code design.

To make the code build and run:<br/> 
	1) Rename View.pro to Ludo.pro<br/> 
	2) In Ludo.pro change Target = View to Target = Ludo<br/> 
	3) In game.cpp method run() out comment emit close();<br/> 
	4) In ludo_player_random.cpp add #include \<random\><br/> 

To expose the bugs:<br/>  
	1) Download the test_game.h and test_game.cpp files from this repository<br/> 
	2) Add test_game.h and test_game.cpp to your Ludo.pro file<br/> 
	3) Add #include "test_game.h" in main.cpp<br/> 
	4) In game.h changed form this<br/> 
		    class game : public QThread<br/> 
		    {<br/> 
	   into this instead<br/> 
		    class test_game;<br/> 
		    class game : public QThread<br/> 
		    {<br/> 
    			friend class test_game;<br/> 
	5) In main.cpp just before g.start(); add<br/> 
		    test_game test;<br/> 
		    test.run_all_tests();<br/> 
  6) In game.cpp game::reset() change this:<br/> 
		    for(auto i : player_positions){<br/> 
	        	i = -1; }<br/> 
     into this instead<br/> 
        for(int i = 0; i < player_positions.size(); i++)<br/> 
        		player_positions[i] = -1;<br/> 
  7) Build and run program 

Then 6 of the unit tests in test_game.cpp should fail:<br/> 
	1) 'test_rel_pos_outfield_special'<br/> 
      Wrong absolute to relative position transformation of a piece on absolute square 51.<br/> 
	2) 'test_rel_pos_goal_stretch'<br/> 
      Wrong absolute to relative position transformation of the pieces on the goal stretch for the players before you.<br/> 
      That is Player_0's pieces for player_1, Player_0 and Player_1's pieces for player_2,<br/> 
      Player_0, Player_1 and Player_2's pieces for player_3<br/> 
	3) 'test_move_piece_send_home'<br/> 
		  For each player_0, player_1 and player_2, there is one of the opponents pieces you can't send home.<br/> 
	4) 'test_move_piece_two_on_globe'<br/> 
		  You send yourself home if you land a second piece on a globe square<br/> 
	5) 'test_move_piece_three_on_square'<br/> 
		  You send yourself home if you land a third piece on a normal square<br/> 
	6) 'test_move_piece_enforce_legal_move'<br/> 
		  You can avoid making a move even if you have legal move options<br/> 

A quick sanity check is to have 4 'ludo_player_ramdon' players play many games against each other.<br/> 
If they ecah take turn starting they are all expected to win 25% of the time. <br/> 
For me player_0 only wins ~ 18% and player_3 wins ~ 40% of the games, showing a significant bias.<br/> 

Code fixes for the bugs and bad software design will be uploaded shortly.
