#include <algorithm>
#include <array>
#include <chrono>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <string>
#include <vector>

using namespace std;

enum Symbol : char {
    Wall = '#',
    Empty = ' ',
    Flag = '$',
    Path = '*',
    Conflict = '@'
};

enum Move : char {
    Up = 'U',
    Down = 'D',
    Left = 'L',
    Right = 'R',
    None = ' '
};

constexpr std::array<Move, 4> MOVES = { Move::Up, Move::Down, Move::Left, Move::Right };

template <typename T>
using Matrix = std::vector<std::vector<T>>;

class Node {
public:
    int row;
    int col;
    Node(int row, int col)
        : row(row)
        , col(col)
    {
    }

    Node move(Move move) const
    {
        int row = this->row;
        int col = this->col;
        switch (move) {
        case Move::Up: row--; break;
        case Move::Down: row++; break;
        case Move::Left: col--; break;
        case Move::Right: col++; break;
        default: break;
        }
        return Node(row, col);
    }

    friend inline bool operator==(const Node& a, const Node& b)
    {
        return a.row == b.row && a.col == b.col;
    };

    friend inline bool operator<(const Node& a, const Node& b)
    {
        return a.row < b.row || (a.row == b.row && a.col < b.col);
    };
};

Move get_opposite_move(const Move move)
{
    switch (move) {
    case Move::Up: return Move::Down;
    case Move::Down: return Move::Up;
    case Move::Left: return Move::Right;
    case Move::Right: return Move::Left;
    default: return Move::None;
    }
}

void print_grid(const std::vector<std::string>& maze)
{
    for (const std::string& line : maze) {
        std::cout << line << '\n';
    }
    std::cout << std::endl;
}

std::string solve(const int COLS, const int ROWS, const std::vector<std::string>& maze)
{
    const Node SRC { 0, 1 };
    const Node DEST { ROWS - 1, COLS - 2 };

    std::vector<Node> nodes;

    nodes.push_back(SRC);
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            if (maze[row][col] == Symbol::Flag) {
                nodes.emplace_back(row, col);
            }
        }
    }
    nodes.push_back(DEST);

    const int NBR_NODES = nodes.size();

    Matrix<std::string> partial_routes(NBR_NODES - 1, std::vector<std::string>(NBR_NODES, ""));

    for (int i = 0; i < NBR_NODES; ++i) {
        const Node& start_node = nodes[i];

        if (start_node == DEST)
            continue;

        Matrix<int> costs(ROWS, std::vector<int>(COLS, INT_MAX));
        Matrix<Move> directions(ROWS, std::vector<Move>(COLS, Move::None));

        static const auto node_is_greater = [&](const Node& a, const Node& b) -> bool {
            return costs[a.row][a.col] > costs[b.row][b.col];
        };

        std::priority_queue<Node, std::vector<Node>, decltype(node_is_greater)> frontier { node_is_greater };

        frontier.push(start_node);
        costs[start_node.row][start_node.col] = 0;

        while (!frontier.empty()) {
            const Node curr_node = frontier.top();
            frontier.pop();

            const int new_cost = costs[curr_node.row][curr_node.col] + 1;

            for (const Move& move : MOVES) {
                const Node next_node = curr_node.move(move);

                if (next_node.row < 0 || next_node.row >= ROWS || next_node.col < 0 || next_node.col >= COLS
                    || maze[next_node.row][next_node.col] == Symbol::Wall)
                    continue;

                if (new_cost < costs[next_node.row][next_node.col]) {
                    costs[next_node.row][next_node.col] = new_cost;
                    directions[next_node.row][next_node.col] = move;

                    frontier.push(next_node);
                }
            }
        }

        for (int j = 0; j < NBR_NODES; ++j) {
            const Node& end_node = nodes[j];

            if (start_node == end_node || end_node == SRC)
                continue;

            Node pos = end_node;
            int cost = costs[pos.row][pos.col];

            if (cost == INT_MAX)
                continue; // impossible to get package, skip

            std::string partial_route;
            partial_route.resize(cost);

            for (auto it = partial_route.rbegin(); it < partial_route.rend(); ++it) {
                const Move move = directions[pos.row][pos.col];
                *it += move;
                pos = pos.move(get_opposite_move(move));
            };

            partial_routes[i][j] = partial_route;
        }
    }

    std::vector<int> node_order(NBR_NODES);
    std::iota(node_order.begin(), node_order.end(), 0);

    auto package_begin = node_order.begin() + 1;
    auto package_end = node_order.end() - 1;

    std::string best_route;
    int min_cost = INT_MAX;

    do {
        std::string route;
        int cost = 0;

        // no std::span bfo
        auto first = node_order.begin();
        auto second = node_order.begin() + 1;

        do {
            std::string& partial_route = partial_routes[*first][*second];
            route += partial_route;
            cost += partial_route.length();

            first++;
            second++;
        } while (second < node_order.end());

        if (cost < min_cost) {
            best_route = route;
            min_cost = cost;
        }
    } while (std::next_permutation(package_begin, package_end));

    return best_route;
}

// DO NOT MODIFY AFTER THIS LINE / NE PAS MODIFIER APRES CETTE LIGNE
string CONSOLE_LANGUAGE = "ENG";

void correction(const int WIDTH, const int HEIGHT, vector<string> maze, const string& path)
{
    int x = 1;
    int y = 0;
    int count = 0;
    bool conflicts = false;
    bool flagReached = false;
    string errorText;

    maze[0].replace(1, 1, 1, Path);

    // go over the path to add it to the maze / itere sur le chemin pour l'ajouter au labyrinthe
    for (const auto& move : path) {
        // movement / deplacement
        switch (move) {
        case Up: y--; break;
        case Down: y++; break;
        case Left: x--; break;
        case Right: x++; break;
        default:
            cout << (CONSOLE_LANGUAGE == "FR" ? "Symbole invalide de deplacement" : "Invalid symbol for the movement") << '\n';
        }

        // check if in maze / regarde si on est dans le labyrinthe
        if (x < 0 || x >= WIDTH) {
            cout << (CONSOLE_LANGUAGE == "FR" ? "Position invalide en largeur" : "Invalid position for the width") << endl;
            break;
        }
        if (y < 0 || y >= HEIGHT) {
            cout << (CONSOLE_LANGUAGE == "FR" ? "Position invalide en hauteur" : "Invalid position for the height") << endl;
            break;
        }

        // Add path or conflict to the maze / Ajoute le chemin ou la collision au labyrinthe
        switch (maze[y][x]) {
        case Empty:
            maze[y].replace(x, 1, 1, Path);
            break;
        case Flag:
            flagReached = true;
        case Path:
        case Conflict:
            break;
        case Wall:
            maze[y].replace(x, 1, 1, Conflict);
            conflicts = true;
            break;
        default:
            cout << (CONSOLE_LANGUAGE == "FR" ? "Symbole inconnu dans le labyrinthe" : "Unknown symbol in the maze") << endl;
            return;
        }
        count++;
    }

    // output for the test / sortie du test
    cout << (CONSOLE_LANGUAGE == "FR" ? "Votre nombre de déplacements est: " : "Your number of moves is: ") << count << '\n'
         << (CONSOLE_LANGUAGE == "FR" ? "Voici le labyrinthe obtenu, les * sont votre chemin et les @ sont des conflits avec les murs" : "This is the maze with your path indicated as * and the conflicts indicated as @") << '\n';

    print_grid(maze);

    if (conflicts) {
        cout << (CONSOLE_LANGUAGE == "FR" ? "Il y a une erreur dans votre parcours" : "There is something problematic in your solution") << '\n'
             << endl;
        return;
    }
    if (flagReached) {
        cout << (CONSOLE_LANGUAGE == "FR" ? "Bravo vous avez atteint le paquet" : "Congratulations you have picked up the package") << '\n'
             << endl;
    }
    if (x == WIDTH - 2 && y == HEIGHT - 1) {
        cout << (CONSOLE_LANGUAGE == "FR" ? "Bravo vous avez atteint la sortie du labyrinthe" : "Congratulations you have reached the end of the maze") << '\n'
             << endl;
    }
}

int main()
{
    const int nbr_cases = 3;
    vector<vector<string>> mazes(nbr_cases);

    // clang-format off
    mazes[0] = { "# #######################",
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
                 "####################### #" };

    mazes[1] = { "# #############",
                 "#   # #     # #",
                 "# # # ### # # #",
                 "# # #     # # #",
                 "# # ##### # ###",
                 "#             #",
                 "# ### ### ### #",
                 "# #$  #   #   #",
                 "# ### # # #####",
                 "#             #",
                 "############# #" };

    mazes[2] = { "# ###################",
                 "#   #          $    #",
                 "# # # ### ### ### # #",
                 "#           # # #   #",
                 "########### # # ### #",
                 "#    $# #   #       #",
                 "# ### # ### ##### ###",
                 "#   # #   #     #   #",
                 "### # ### ### # #####",
                 "#     #     # # # # #",
                 "#$### ### # # # # # #",
                 "# # # #   #         #",
                 "# # # ### ######### #",
                 "#                   #",
                 "################### #" };

    // clang-format on

    using Clock = chrono::high_resolution_clock;
    using Ms = chrono::duration<float, chrono::milliseconds::period>;

    vector<bool> should_test { true, true, true };

    Ms total_duration;

    for (int i = 0; i < nbr_cases; i++) {
        if (!should_test[i])
            continue;

        cout << "\nTest #" << i + 1 << endl;

        auto start = Clock::now();
        const string answer = solve(mazes[i][0].length(), mazes[i].size(), mazes[i]);
        Ms duration = Clock::now() - start;
        total_duration += duration;

        cout << std::fixed << std::setprecision(3)
             << (CONSOLE_LANGUAGE == "FR" ? "Temps de Résolution: " : "Solve Time: ")
             << duration.count() << "ms"
             << "\n\n";

        correction(mazes[i][0].length(), mazes[i].size(), mazes[i], answer);
    }

    cout << std::fixed << std::setprecision(3)
         << (CONSOLE_LANGUAGE == "FR" ? "Temps de Résolution Totale: " : "Total Solve Time: ")
         << total_duration.count() << "ms"
         << "\n\n";
}
