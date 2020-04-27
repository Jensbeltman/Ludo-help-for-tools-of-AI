cmake_minimum_required(VERSION 3.15)
project(Domain_Code)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(Domain_Code
        dice.h
        game.cpp
        game.h
        iplayer.h
        main.cpp
        player_random.h
        positions_and_dice.h
        test_game.cpp
        test_game.h)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(Domain_Code Threads::Threads)

find_package( Boost REQUIRED COMPONENTS program_options regex )
include_directories( ${Boost_INCLUDE_DIRS} )

# Alternatively you could use ${Boost_LIBRARIES} here.
target_link_libraries( Domain_Code ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_REGEX_LIBRARY} )