#include <iostream>
#include <string>
#include <vector>
using namespace std;
string CONSOLE_LANGUAGE = "ENG";
//L'anglais est mis par defaut. Si vous voulez un affichage en francais, changez le string de la ligne precedente pour "FR"


string solve(const int WIDTH, const int HEIGHT, const vector<string> maze) {
	string answer;
	// Your code goes here




	return answer;
}


//DO NOT MODIFY AFTER THIS LINE / NE PAS MODIFIER APRES CETTE LIGNE
enum Symbol {
	Wall = '#',
	Empty = ' ',
	Flag = '$',
	Path = '*',
	Conflict = '@'
};
enum Move {
	Up = 'U',
	Down = 'D',
	Left = 'L',
	Right = 'R'
};

void printgrid(const vector<string>& maze) {
	for (const string line : maze) {
		cout << line << endl;
	}
}

void correction(const int WIDTH, const int HEIGHT, vector<string> maze, const string& path) {
	int x = 1;
	int y = 0;
	int count = 0;
	bool conflicts = false;
	bool flagReached = false;
	string errorText;

	maze[0].replace(1, 1, 1, char(Path));
	
	// go over the path to add it to the maze / itere sur le chemin pour l'ajouter au labyrinthe
	for (int i=0; i < path.length(); i++) {
		// movement / deplacement
		switch(path[i]){
			case Up:
				y--;
				break;
			case Down:
				y++;
				break;
			case Left:
				x--;
				break;
			case Right:
				x++;
				break;
			default:
				errorText = CONSOLE_LANGUAGE == "FR" ? "Symbole invalide de deplacement" : "Invalid symbol for the movement";
				cout << errorText << endl;
		}

		// check if in maze / regarde si on est dans le labyrinthe
		if (x < 0 || x >= WIDTH) {
			errorText = CONSOLE_LANGUAGE == "FR" ? "Position invalide en largeur" : "Invalid position for the width";
			cout << errorText << endl;
			break;
		}
		if (y < 0 || y >= HEIGHT) {
			errorText = CONSOLE_LANGUAGE == "FR" ? "Position invalide en hauteur" : "Invalid position for the height";
			cout << errorText << endl;
			break;
		}

		// Add path or conflict to the maze / Ajoute le chemin ou la collision au labyrinthe
		switch (maze[y][x]){
			case Empty:
				maze[y].replace(x, 1, 1, char(Path));
				break;
			case Flag:
				flagReached = true;
			case Path:
			case Conflict:
				break;
			case Wall:
				maze[y].replace(x, 1, 1, char(Conflict));
				conflicts = true;
				break;
			default:
				errorText = CONSOLE_LANGUAGE == "FR" ? "Symbole inconnu dans le labyrinthe" : "Unknown symbol in the maze";
				cout << errorText << endl;
				return;
		}
		count++;
	}

	// output for the test / sortie du test
	string out = CONSOLE_LANGUAGE == "FR" ? "Votre nombre de déplacements est: " : "Your number of moves is: ";
	cout << out << count << endl;
	out = CONSOLE_LANGUAGE == "FR" ? "Voici le labyrinthe obtenu, les * sont votre chemin et les @ sont des conflits avec les murs" : "This is the maze with your path indicated as * and the conflicts indicated as @";
	cout << out << endl;
	
	printgrid(maze);

	if(conflicts) {
		out = CONSOLE_LANGUAGE == "FR" ? "Il y a une erreur dans votre parcours" : "There is something problematic in your solution";
		cout << out << endl << endl;
		return;
	} 
	if (flagReached) {
		out = CONSOLE_LANGUAGE == "FR" ? "Bravo vous avez atteint le paquet" : "Congratulations you have picked up the package";
		cout << out << endl << endl;
	}
	if (x == WIDTH-2 && y == HEIGHT-1) {
		out = CONSOLE_LANGUAGE == "FR" ? "Bravo vous avez atteint la sortie du labyrinthe" : "Congratulations you have reached the end of the maze";
		cout << out << endl << endl;
	} 
	
}


int main(int argc, char *argv[]){
	const int nbr_cases = 3;
	vector<vector<string>> MAZES (nbr_cases);
	vector<string> answers (nbr_cases);
	MAZES[0] = {"# #######################",
                    "#   #   #               #",
                    "# # # # # # # ### # ### #",
                    "# # # # #       # #   # #",
                    "# # # # ####### # ### ###",
                    "#           # # #  $    #",
                    "# ### ### # # # # ### # #",
                    "# #   #   #         # # #",
                    "# ### ### ######### # ###",
                    "#   # # #   #   #   #   #",
                    "### # # # # ### # # # # #",
                    "#     # # #     # # # # #",
                    "# ### # # # ### # # # # #",
                    "#           # # #       #",
                    "# ### # # # # # # #######",
                    "# #   #           # # # #",
                    "# ### ########### # # # #",
                    "#     # #   #   #   #   #",
                    "# ### # # # # # # # # ###",
                    "#                       #",
                    "####################### #"};

	MAZES[1] = {"# #############",
                    "#   # #     # #",
                    "# # # ### # # #",
                    "# # #     # # #",
                    "# # ##### # ###",
                    "#             #",
                    "# ### ### ### #",
                    "# #$  #   #   #",
                    "# ### # # #####",
                    "#             #",
                    "############# #"};

	MAZES[2] = {"# ###################",
                    "#   #               #",
                    "# # # ### ### ### # #",
                    "#           # # #   #",
                    "########### # # ### #",
                    "#     # #   #       #",
                    "# ### # ### ##### ###",
                    "#   # #   #     #   #",
                    "### # ### ### # #####",
                    "#     #     # # # # #",
                    "#$### ### # # # # # #",
                    "# # # #   #         #",
                    "# # # ### ######### #",
                    "#                   #",
                    "################### #"};

	for (int i=0; i<nbr_cases; i++) {
		answers[i] = solve(MAZES[i][0].length(), MAZES[i].size(), MAZES[i]);
	}
	for (int i=0; i<nbr_cases; i++) {
		cout << endl <<"Test #" << i+1 << endl;
		correction(MAZES[i][0].length(), MAZES[i].size(), MAZES[i], answers[i]);
	}
}
