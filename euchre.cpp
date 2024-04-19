#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "Player.hpp"
#include "Pack.hpp"


class Game 
{
private:
    int points_to_win;
    int team_1_points;
    int team_2_points;
    int team_ordered_up;
    int hand_total;
    bool shuffle_boolean;
    bool trump_called;
    bool run_game;
    Suit trump;
    Card up_card;
    Card winning_card;
    Card led_card;
    Card follow_up;
    Pack pack;
    std::vector<Player*> players;

public:

    Game(int points_to_win, Pack pack_in, const std::vector<std::string> &specs)
        : points_to_win(points_to_win), team_1_points(0), team_2_points(0), 
        team_ordered_up(0), hand_total(0), shuffle_boolean(false), 
        trump_called(false), run_game(true), pack(pack_in)
    {
        // initialize players vector
        for (int i = 1; i < 9; i += 2)
        {
            players.push_back(Player_factory(specs[i], specs[i + 1]));

        }

        // initialize shuffle
        if (specs[0] == "shuffle") 
        { 
            shuffle_boolean = true; 
        }
    }

    void play() 
    {
        while(run_game) {
            // doesnt erase previous hand? so the assertion fails
            for (int dealer_index = 0; dealer_index <= 3; dealer_index++) 
            {
                shuffle(shuffle_boolean);

                deal(dealer_index);

                trump_called = false;
                for (int round = 1; round <= 2; round++) {
                    make_trump(dealer_index, up_card, round);
                }

                play_hand(dealer_index);
                hand_total++;

                if (team_1_points >= points_to_win || team_2_points >= points_to_win) {
                    run_game = false;
                    break; //end game the moment a hand results in a winning score
                }
            }
        }
        if (team_2_points<team_1_points) {
            std::cout <<  *players[0] << " and " << *players[2] << " win!" << std::endl;
        }
        else if (team_1_points<team_2_points) {
            std::cout <<  *players[1] << " and " << *players[3] << " win!" << std::endl;
        }

        for (size_t i = 0; i < players.size(); ++i) {
            delete players[i];
        }

    }

    void shuffle(bool shuf) 
    {
        pack.reset();
        if (shuf) {
            pack.shuffle();
        }
    }

    void deal(int d_index) 
    {
        Card card_being_dealt(pack.deal_one());

        for(int p_index = d_index+1, batch = 0; 
            p_index < d_index + 9; p_index++, batch++)
        {
            players[p_index%4]->add_card(card_being_dealt);

            card_being_dealt = pack.deal_one();

            players[p_index%4]->add_card(card_being_dealt);

            card_being_dealt = pack.deal_one();

            if (batch == 0 || batch == 2 
                || batch == 5 || batch == 7) 
            {   
                //deal third card if necessary
                players[p_index%4]->add_card(card_being_dealt);
                card_being_dealt = pack.deal_one();
            }
        } 

        // card_being_dealt = pack.deal_one();

        up_card = card_being_dealt;

        print_start(d_index);
    }


    void make_trump(int d_index, Card up_card, int round) 
    {
        for (int p_index = d_index+1; p_index <= d_index+4; p_index++) 
        {
            if (!trump_called && 
               players[p_index%4]->make_trump(up_card,p_index%4==d_index,round,trump))
                {
                std::cout << *players[p_index%4] << " orders up " 
                          << trump << std::endl << std::endl;
                trump_called = true;
                //if round one, then dealer swaps a card
                if (round == 1) {
                    players[d_index]->add_and_discard(up_card);
                }
                //records what team called trump, for scoring purposes
                if (p_index%4 == 0 || p_index%4 == 2) {
                    team_ordered_up = 1;
                }
                else if (p_index%4 == 1 || p_index%4 == 3) {
                    team_ordered_up = 2;
                }
            }
            else if (!trump_called) {
                std::cout << *players[p_index%4] << " passes" << std::endl;
            }

        }
    }
    
    
    void play_hand(int dealer_index) 
    {   
        int leader_index = dealer_index + 1;
        int tricks_taken_1 = 0;
        int tricks_taken_2 = 0;
        for (int trick = 0; trick < 5; trick++) 
        {
            //I think you can just make this the parameter for the play_card
            led_card = players[leader_index%4]->lead_card(trump);
            winning_card = led_card;

            std::cout << led_card << " led by " << *players[leader_index%4] 
                      << std::endl;

            for (int index = leader_index+1, play = 2; play < 5; index++, play++) 
            {
                //card that follows the leader 
                follow_up = players[index%4]->play_card(led_card, trump); 
                
                std::cout << follow_up << " played by " << *players[index%4] 
                          << std::endl;
                
                //checks if follow up card is winning and changes winning card if so
                if (Card_less(winning_card, follow_up, led_card, trump))
                {
                    winning_card = follow_up;
                    leader_index = index%4;
                }
            }
            std::cout << *players[leader_index%4] << " takes the trick" 
                      << std::endl << std::endl;
            //checks what team just won the trick, i.e. the new leader
            if (leader_index%4 == 0 || leader_index%4 == 2) {
                tricks_taken_1++;
            }
            else if (leader_index%4 == 1 || leader_index%4 == 3) {
                tricks_taken_2++;
            }

        }

        if (tricks_taken_2 < tricks_taken_1) {
            std::cout << *players[0] << " and " 
            << *players[2] << " win the hand" << std::endl;

            team_1_points += score_points(tricks_taken_1, 1);
        }
        else if (tricks_taken_1 < tricks_taken_2) {
            std::cout << *players[1] << " and " 
            << *players[3] << " win the hand" << std::endl;

            team_2_points += score_points(tricks_taken_2, 2);
        }

        print_score();
    }

    void print_start(int d_index) {
        std::cout << "Hand " << hand_total << std::endl;
        std::cout << *players[d_index] << " deals" << std::endl;
        std::cout << up_card << " turned up" << std::endl;
    }

    int score_points(int tricks_taken, int team_number) {
        int points_awarded = 1;
        if (tricks_taken == 5 && team_ordered_up == team_number) {
            std::cout << "march!" << std::endl;
            points_awarded = 2;
        }
        else if (team_number != team_ordered_up) {
            std::cout << "euchred!" << std::endl;
            points_awarded = 2;
        }
        
        return points_awarded;
    }

    void print_score() 
    {
        std::cout << *players[0] << " and " << *players[2] 
        << " have " << team_1_points << " points" << std::endl;

        std::cout << *players[1] << " and " << *players[3] 
        << " have " << team_2_points << " points" << std::endl << std::endl;
    }

};

int main(int argc, char **argv) 
{
    std::ifstream input_file;
    bool argument_error = false;
    std::vector<std::string> specs;

    // test arguments for errors
    if ((argc != 12)
        || (std::stoi(argv[3]) < 1 || std::stoi(argv[3]) > 100)
        || (!strcmp(argv[2], "shuffle") && !strcmp(argv[2], "noshuffle")))
    {
        argument_error = true;
    }

    // test strategies arguments for errors and push arguments into vector
    for (int i = 2; i <= 11; i++)
    {
        // push argument to player specifications
        if (i != 3) { specs.push_back(argv[i]); }

        // test if player strategy is valid
        if (i % 2 != 0 && !strcmp(argv[i], "Simple") && !strcmp(argv[i], "Human")) 
        { 
            argument_error = true;
        }
    }

    // output error message if there is an error in arguments
    if (argument_error)
    {
        std::cout << "Usage: euchre.exe PACK_FILENAME [shuffle|noshuffle] "
                  << "POINTS_TO_WIN NAME1 TYPE1 NAME2 TYPE2 NAME3 TYPE3 "
                  << "NAME4 TYPE4" << std::endl;
        return -1;
    }

    // test file for errors
    input_file.open(argv[1]);

    // output error message if there is an error in file opening
    if (!input_file.is_open())
    {
        std::cout << "Error opening " << argv[1] << std::endl;
        return -1;
    }

    // output arguments
    for (int i = 0; i < argc; i++)
    {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;

    // create game object and initiate game
    Game game = Game(std::stoi(argv[3]), input_file, specs);
    game.play();
}