#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <stack>
#include <cmath>
using namespace std;
string CONSOLE_LANGUAGE = "ENG";
//L'anglais est mis par defaut. Si vous voulez un affichage en francais, changez le string de la ligne precedente pour "FR"

bool STOP_TESTS_ON_ERROR = false;
//will stop testing at the first problem in the transactions / permet d'arreter de tester si le programme detecte une mauvaise transaction

#define WIDTH 17
#define HEIGHT 11


string solve(const vector<string> GRID, string time, int move_speed, float tower_speed, int flip_time, int grab_time){
	string answer;
	// Your code goes here





	return answer;
}


//DO NOT MODIFY AFTER THIS LINE / NE PAS MODIFIER APRES CETTE LIGNE
enum Action {
	Right = 'R',
	Left = 'L',
	Down = 'D',
	Up = 'U',
	Flip = 'F',
	Grab = 'G',
	Tower = 'T'
};

vector<tuple<int, int>> getRobots(vector<string> grid) {
	vector<tuple<int, int>> ennemies = {};
	tuple<int, int> your_robot;;
	for (int i=0; i<HEIGHT; i++) {
		for (int j=0; j<WIDTH; j++) {
			if(grid[i].at(j) == '#') {
				ennemies.push_back(make_tuple(j, i));
			} else if (grid[i].at(j) == '&') {
				your_robot = make_tuple(j, i);
			}
		}
	}
	ennemies.push_back(your_robot);
	return ennemies;
}

bool isValidPosition (int posX, int posY, vector<tuple<int, int>> robots) {
	if (posX < 0 || posX >= WIDTH || posY < 0 || posY >= HEIGHT) return false;
	for (int i=0; i<3; i++) {
		if (posX == get<0>(robots[i]) && posY == get<1>(robots[i])) return false;
	}
	return true;
}

tuple<int, int, int> getTowerLevels(vector<string> grid) {
	return make_tuple(stoi(grid[5].substr(4, 1)), stoi(grid[5].substr(8, 1)), stoi(grid[5].substr(12, 1)));
}

tuple<int, int> getTowerScoreTime (int posX, int posY, float speed, tuple<int, int, int>* tower_levels) {
	vector<tuple<int,int>> scoreHeight = {make_tuple(ceil(1.5/speed), 10), make_tuple(ceil(6.0/speed), 20), make_tuple(ceil(14.5/speed), 40), make_tuple(ceil(25.5/speed), 80), make_tuple(ceil(39.0/speed), 160), make_tuple(ceil(55.0/speed), 320), make_tuple(ceil(73.5/speed), 640)};
	if (posY != 5) return make_tuple(0, 0);
	if (posX == 4 && get<0>(*tower_levels) < 5) {
		return scoreHeight[get<0>(*tower_levels)++];
	} else if (posX == 8 && get<1>(*tower_levels) < 6) {
		return scoreHeight[get<1>(*tower_levels)++];
	} else if (posX == 12 && get<2>(*tower_levels) < 5) {
		return scoreHeight[get<2>(*tower_levels)++];
	}
	return make_tuple(0, 0);
}

bool checkAnswer (string result, vector<string> grid, string time_string, int move_speed, float tower_speed, int flip_speed, int grab_time) {
	bool valid = true;
	const bool fr = CONSOLE_LANGUAGE == "FR";
	const int nbr_actions = result.size();
	int time = stoi(time_string.substr(0, 1)) * 60 + stoi(time_string.substr(2, 2));
	vector<tuple<int, int>> robots = getRobots(grid);
	tuple<int, int, int> tower_levels = getTowerLevels(grid);
	int posX = get<0>(robots[3]); 
	int posY = get<1>(robots[3]);
	int move_time = 24 / move_speed; // 2' = 24"
	stack<char> pieces_on;
	int score = 0;
	bool tried_flip;

	string text = fr ? "La position de départ est: " : "The starting position is: ";
	cout << text << posY << " : " << posX << endl;

	if (nbr_actions == 0) {
		text = fr ? "Vous n'avez fourni aucune action" : "You have provided no action";
		cout << text << endl;
		return false;
	}

	for (int i=0; i< nbr_actions; i++) {
		tried_flip = false;
		string action;
		char tile;
		tuple<int, int> tower_score;
		switch (result[i]) {
			case Up:
				action = "Up";
				posY--;
				time -= move_time;
				break;
			case Down:
				action = "Down";
				posY++;
				time -= move_time;
				break;
			case Left:
				action = "Left";
				posX--;
				time -= move_time;
				break;
			case Right:
				action = "Right";
				posX++;
				time -= move_time;
				break;

			case Flip:
				tile = grid[posY].at(posX);
				if (tile == 'J') {
					grid[posY].replace(posX, 1, "B");
					tried_flip = true;
					action = fr ? "Vous avez retourné une pièce de jeu de jaune vers bleu" : "You have flipped a GP from yellow to blue";
					score += 35;
					time -= flip_speed;
				} else if (tile == 'B') {
					grid[posY].replace(posX, 1, "J");
					tried_flip = true;
					action = fr ? "Vous avez retourné une pièce de jeu de bleu vers jaune" : "You have flipped a GP from blue to yellow";
					score -= 35;
					time -= flip_speed;
				} else {
					action = fr ? "Vous essayez de flip un endroit qui ne contient aucune piece" : "You are trying to flip a spot with no game piece";
				}
				break;

			case Grab:
				tile = grid[posY].at(posX);
				if (tile == 'J') {
					grid[posY].replace(posX, 1, " ");
					pieces_on.push('J');
					action = fr ? "Vous avez pris une pièce de jeu jaune dans votre robot" : "You have picked up a yellow game piece";
					time -= grab_time;
				} else if (tile == 'B') {
					grid[posY].replace(posX, 1, " ");
					pieces_on.push('B');
					action = fr ? "Vous avez pris une pièce de jeu bleu dans votre robot" : "You have picked up a blue game piece";
					score -= 35;
					time -= grab_time;
				} else {
					action = fr ? "Vous avez essayer de prendre a un endroit qui ne contient aucun GP" : "You have tried to to grab a spot with no GP";
				}
				break;

			case Tower:
				action = "Tower";
				if (posY != 5 || !(posX == 4 || posX == 8 || posX == 12)) {
					action = fr ? "Vous avez essayer de mettre une piece alors que vous n'etes pas a une tour" : "You have tried to put a game piece but you are not at a tower";
					break;
				}
				tower_score = getTowerScoreTime(posX, posY, tower_speed, &tower_levels);
				if (get<0>(tower_score) == 0) {
					text = fr ? "Vous essayer de faire un placement invalide dans la tour" : "You have tried to do an invalid tower placement";
					valid = false;
					break;
				}
				if (pieces_on.empty()) {
					action = fr ? "Vous avez essayer de mettre une piece dans la tour alors que vous n'en avez pas" : "You have tried to put a game piece in the tower but you don't have any on your robot";
					break;
				}
				if (pieces_on.top() == 'J') {
					action = fr ? "Vous avez mis une piece de jeu de l'adversaire dans la tour" : "You have put a game piece of the opposing team in the tower";
					time -= get<0>(tower_score);
				} else {
					action = fr ? "Vous avez mis une piece de jeu dans la tour" : "You have put a game piece in the tower";
					time -= get<0>(tower_score);
					score += get<1>(tower_score);
				}
				pieces_on.pop();
				break;
			default:
				text = fr ? "Vous avez une action invalide" : "You have an invalid action";
				valid = false;
		}

		//validity checks
		if (tried_flip && pieces_on.size() >= 2) {
			valid = false;
			text = fr ? "Il est interdit de faire un flip avec deux GP dans le robot" : "It is not allowed to make a flip with 2 GP in your robot";
		}
		if (pieces_on.size() > 2) {
			valid = false;
			text = fr ? "Vous ne pouvez pas transporter autant de pieces de jeu" : "You can't carry this amount of game pieces";
		}
		if (!isValidPosition(posX, posY, robots)) {
			valid = false;
			text = fr ? "Position invalide" : "Invalid Position";
		}
		if (time < 0) {
			valid = false;
			text = fr ? "Vous avez dépassé le temps limite" : "You have exceeded the time available";
		}
		if (!valid) break;

		cout << "Pos: (" << posY << "," << posX << ") time: " << time <<  "s score: " << score << " action: " << action << endl;
	}
	if (!valid) cout << endl << "STOP" << endl << text << endl << endl;
	else {
		text = fr ? "Voici le temps restant et le score total: " : "Here is the time left and the total score: ";
		cout << endl << text << time << "s & " << score << " points" << endl << endl;
	}
	return valid;
}

int main(int argc, char *argv[]){
	const int nbr_cases = 5;
	vector<vector<string>> grids (nbr_cases);
	vector<string> time_left (nbr_cases);
	vector<int> move_speed (nbr_cases);
	vector<float> tower_speed (nbr_cases);
	vector<int> flip_time (nbr_cases);
	vector<int> grab_time (nbr_cases);
	vector<string> result = {};
	bool passed = true;

	grids[0] = {"         BJ      ",
				"      BJ  JJ J   ",
				"   J   BBB    J  ",
				"   &#JJ   B  JJ  ",
				"   BB    BJ   B  ",
				" B  2 J 7   5    ",
				"   J J   # JB J  ",
				"   #  BJ BJ  J   ",
				" J   BJ    JBB   ",
				"     J  J   JJJ  ",
				"            J    "};
	time_left[0] = "1:31";
	move_speed[0] = 24;
	tower_speed[0] = 20;
	flip_time[0] = 2;
	grab_time[0] = 1;

	grids[1] = {"      J JJ  J    ",
				"   J   B J  JB   ",
				"  &B J     J  #J ",
				"  J    J B   J J ",
				"  B J #B  JJ  B  ",
				"  J 2 B 1B  3  JB",
				"J     J   J   J  ",
				"  B J  J #   B J ",
				"  B   JB J B    J",
				"   JJ  J  BJ  B  ",
				"      BJ   JJ    "};
	time_left[1] = "3:12";
	move_speed[1] = 12;
	tower_speed[1] = 7.7;
	flip_time[1] = 2;
	grab_time[1] = 2;

	grids[2] = {"     B   J J     ",
				" B  BJJ    J B   ",
				"   BJ#JJ  JB  B  ",
				"   J  JJB  JB    ",
				"     J     J  B  ",
				" J  1  B0 JJ0 J  ",
				"   BJ  J    J #  ",
				"  JB  JJ JJB  BBB",
				"   &BBB   JB  #  ",
				"  J   J  JJ   B  ",
				"     J BJ  JJ    "};
	time_left[2] = "3:46";
	move_speed[2] = 8;
	tower_speed[2] = 11.11;
	flip_time[2] = 3;
	grab_time[2] = 2;

	grids[3] = {"    J  B         ",
				"     JB    JB    ",
				"   J  J  BJJ     ",
				" J  B  BJ  BJ    ",
				"  J J J #   B  J ",
				"J JB4  J3# &6    ",
				" J  J   #  J    J",
				"    B  J    JB   ",
				"    JBJ   BJ     ",
				"     J  JJ JB B  ",
				"       J    J    "};
	time_left[3] = "1:58";
	move_speed[3] = 12;
	tower_speed[3] = 8.0;
	flip_time[3] = 1;
	grab_time[3] = 3;	

	grids[4] = {"    JB    B      ",
				"     JJ    BB    ",
				"BJ  BB  JJ  J    ",
				"   JJJ  BJ    B  ",
				"     BJ   #JJB   ",
				"  J 0  #5   1JJ  ",
				" B   B B#  J  JB ",
				"   J J J    J    ",
				"  JB  J  B J&J   ",
				"      BJBB  J  J ",
				"     J    BB     "};
	time_left[4] = "4:02";
	move_speed[4] = 6;
	tower_speed[4] = 31.3;
	flip_time[4] = 3;
	grab_time[4] = 4;	

	//test cases / cas de test
	for (int i=0; i<nbr_cases; i++){
		result.push_back(solve(grids[i], time_left[i], move_speed[i], tower_speed[i], flip_time[i], grab_time[i]));
	}
	//verification of the profit made / verification du profit fait
	for (int i=0; i<nbr_cases; i++) {
		cout << endl << "Test " << (i+1) << endl << endl;
		passed &= checkAnswer(result[i], grids[i], time_left[i], move_speed[i], tower_speed[i], flip_time[i], grab_time[i]);
		if (STOP_TESTS_ON_ERROR && !passed) break;
	}

	//final output / affichage final
	string output;
	if (passed) output = CONSOLE_LANGUAGE == "FR" ? "Tout a l'air bon!\n" : "Everything seems good!\n";
	else output = CONSOLE_LANGUAGE == "FR" ? "Il semble y avoir un problème avec certaines sorties.\n" : "There seems to be a problem in the outputs somewhere.\n";
	cout << endl << output;
}
