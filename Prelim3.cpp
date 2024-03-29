#include <algorithm>
#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <queue>
#include <string>
#include <vector>

enum Symbol : char {
    WALL = '#',
    EMPTY = ' ',
    FLAG = '$',
    PATH = '*',
    CONFLICT = '@'
};

enum Move : char {
    UP = 'U',
    DOWN = 'D',
    LEFT = 'L',
    RIGHT = 'R',
    NONE = ' '
};

constexpr std::array<Move, 4> NON_TRIVIAL_MOVES { Move::UP, Move::DOWN, Move::LEFT, Move::RIGHT };

template <typename T>
using Matrix = std::vector<std::vector<T>>;

struct Node {
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
        int new_row = this->row;
        int new_col = this->col;

        switch (move) {
        case Move::UP: new_row--; break;
        case Move::DOWN: new_row++; break;
        case Move::LEFT: new_col--; break;
        case Move::RIGHT: new_col++; break;
        default: break;
        }

        return { new_row, new_col };
    }

    friend inline bool operator==(const Node& a, const Node& b)
    {
        return a.row == b.row && a.col == b.col;
    };
};

Move get_opposite_move(const Move move)
{
    switch (move) {
    case Move::UP: return Move::DOWN;
    case Move::DOWN: return Move::UP;
    case Move::LEFT: return Move::RIGHT;
    case Move::RIGHT: return Move::LEFT;
    default: return Move::NONE;
    }
}

std::string solve(const int COLS, const int ROWS, const std::vector<std::string>& maze)
{
    // I just realized there will always only be one package (bruh) but too lazy to change it
    // so here is an implementation that works with n packages

    const Node SRC { 0, 1 };
    const Node DEST { ROWS - 1, COLS - 2 };

    std::vector<Node> nodes;

    nodes.push_back(SRC);

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            if (maze[row][col] == Symbol::FLAG) {
                nodes.emplace_back(row, col); // add packages into nodes
            }
        }
    }

    nodes.push_back(DEST);

    const size_t NBR_NODES = nodes.size();

    // 2d vector storing the routes between each of the nodes: partial_routes[start_node][end_node]
    // it has NBR_NODES - 1 rows because DEST is always the last node, i.e. cannot go from DEST to any other node
    Matrix<std::string> partial_routes(NBR_NODES - 1, std::vector<std::string>(NBR_NODES, ""));

    // instead of running path-finding for each node pair, we generate a cost table and a direction table for a node using BFS,
    // then we can simply look up the optimal route from the node to all other nodes (potentially faster than A*?)
    for (size_t i = 0; i < NBR_NODES; ++i) {
        const Node& start_node = nodes[i];

        if (start_node == DEST) // DEST will always be the last node, so we cannot go from DEST to another node
            continue;

        Matrix<unsigned int> costs(ROWS, std::vector<unsigned int>(COLS, UINT_MAX));
        Matrix<Move> directions(ROWS, std::vector<Move>(COLS, Move::NONE)); // stores the direction that a given node came from

        // custom comparator which prioritizes squares with lower cost 
        // note that using operator> causes the priority queue to act as a min priority queue and vice versa (weird as hell)
        static const auto compare_node = [&costs](const Node& lhs, const Node& rhs) -> bool {
            return costs[lhs.row][lhs.col] > costs[rhs.row][rhs.col];
        };

        // prioritize processing squares with lower cost (min priority queue)
        std::priority_queue<Node, std::vector<Node>, decltype(compare_node)> frontier { compare_node };

        // add src node to queue
        frontier.push(start_node);
        costs[start_node.row][start_node.col] = 0;

        while (!frontier.empty()) {
            const Node curr_node = frontier.top();
            frontier.pop();

            const unsigned int new_cost = costs[curr_node.row][curr_node.col] + 1;

            for (const Move& move : NON_TRIVIAL_MOVES) {
                const Node next_node = curr_node.move(move);

                if (next_node.row < 0 || next_node.row >= ROWS || next_node.col < 0 || next_node.col >= COLS // out of bounds
                    || maze[next_node.row][next_node.col] == Symbol::WALL // square is a wall, do not process
                )
                    continue;

                if (new_cost < costs[next_node.row][next_node.col]) {
                    // update value of square or a shortcut is found
                    costs[next_node.row][next_node.col] = new_cost;
                    directions[next_node.row][next_node.col] = move;

                    frontier.push(next_node);
                }
            }
        }

        // now, reconstruct partial route from start_node to other nodes
        for (size_t j = 0; j < NBR_NODES; ++j) {
            const Node& end_node = nodes[j];

            if (start_node == end_node // cannot go to same node
                || end_node == SRC // SRC is always the first node, so we cannot go from any node back to SRC
            )
                continue;

            Node pos = end_node;
            const unsigned int cost = costs[pos.row][pos.col];

            if (cost == UINT_MAX)
                continue; // impossible to get to node, skip

            std::string partial_route;
            partial_route.resize(cost); // route will be of length cost

            // reconstruct partial route by backtracking from end point to start point
            for (auto rit = partial_route.rbegin(); rit < partial_route.rend(); ++rit) {
                const Move move = directions[pos.row][pos.col];
                *rit = move;
                pos = pos.move(get_opposite_move(move)); // go backwards
            }

            partial_routes[i][j] = partial_route; // store partial route
        }
    }

    // time to reconstruct and find optimal route (travelling salesman)

    std::vector<int> node_order(NBR_NODES); // based on the ordering of the nodes vector (ugly to next_permutation directly on nodes vector)
    std::iota(node_order.begin(), node_order.end(), 0); // {0, 1, 2, ... , NBR_NODES - 1 , NBR_NODES} <-- similar to python range

    // iterators for packages, needed to permutation order of packages
    auto packages_begin = node_order.begin() + 1;
    auto packages_end = node_order.end() - 1;

    std::string best_route;
    unsigned int min_cost = UINT_MAX;

    do {
        std::string route;
        unsigned int cost = 0;

        // sliding window (is there better way?)
        auto first = node_order.begin();
        auto second = node_order.begin() + 1;

        while (second < node_order.end()) {
            const std::string& partial_route = partial_routes[*first][*second];
            route += partial_route;
            cost += partial_route.length();

            first++;
            second++;
        }

        if (cost < min_cost) {
            // new best route
            best_route = route;
            min_cost = cost;

            // std::cout << cost << ' ' << route << '\n';
        }
    } while (std::next_permutation(packages_begin, packages_end)); // only need to permutation packages

    return best_route; // yay
}

// DO NOT MODIFY AFTER THIS LINE / NE PAS MODIFIER APRES CETTE LIGNE
using namespace std;
const string CONSOLE_LANGUAGE = "ENG"; // "ENG" | "FR"

void print_grid(const std::vector<std::string>& maze)
{
    for (const std::string& line : maze) {
        std::cout << line << '\n';
    }
    std::cout << std::endl;
}

void correction(const int WIDTH, const int HEIGHT, vector<string> maze, const string& path)
{
    int x = 1;
    int y = 0;
    int count = 0;
    bool conflicts = false;
    bool flagReached = false;
    string errorText;

    maze[0].replace(1, 1, 1, PATH);

    // go over the path to add it to the maze / itere sur le chemin pour l'ajouter au labyrinthe
    for (const auto& move : path) {
        // movement / deplacement
        switch (move) {
        case UP: y--; break;
        case DOWN: y++; break;
        case LEFT: x--; break;
        case RIGHT: x++; break;
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
        case EMPTY:
            maze[y].replace(x, 1, 1, PATH);
            break;
        case FLAG:
            flagReached = true;
        case PATH:
        case CONFLICT:
            break;
        case WALL:
            maze[y].replace(x, 1, 1, CONFLICT);
            conflicts = true;
            break;
        default:
            cout << (CONSOLE_LANGUAGE == "FR" ? "Symbole inconnu dans le labyrinthe" : "Unknown symbol in the maze") << endl;
            return;
        }
        count++;
    }

    // output for the test / sortie du test
    cout << (CONSOLE_LANGUAGE == "FR" ? "Votre nombre de déplacements est: " : "Your number of moves is: ") << count << "\n"
         << (CONSOLE_LANGUAGE == "FR" ? "Voici le labyrinthe obtenu, les * sont votre chemin et les @ sont des conflits avec les murs" : "This is the maze with your path indicated as * and the conflicts indicated as @") << '\n';

    print_grid(maze);

    if (conflicts) {
        cout << (CONSOLE_LANGUAGE == "FR" ? "Il y a une erreur dans votre parcours" : "There is something problematic in your solution") << endl;
        return;
    }
    if (flagReached) {
        cout << (CONSOLE_LANGUAGE == "FR" ? "Bravo vous avez atteint le paquet" : "Congratulations you have picked up the package") << endl;
    }
    if (x == WIDTH - 2 && y == HEIGHT - 1) {
        cout << (CONSOLE_LANGUAGE == "FR" ? "Bravo vous avez atteint la sortie du labyrinthe" : "Congratulations you have reached the end of the maze") << endl;
    }
}

int main()
{
    constexpr int nbr_cases = 3;
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
                 "################### #" };

    // clang-format on

    using Clock = chrono::high_resolution_clock;
    using Ms = chrono::duration<float, chrono::milliseconds::period>;

    // debug purposes
    array<bool, nbr_cases> should_test { true, true, true };

    Ms total_duration;

    for (int i = 0; i < nbr_cases; i++) {
        if (!should_test[i])
            continue;

        cout << "\nTest #" << i + 1 << endl;

        auto start = Clock::now();

        const string answer = solve(
            static_cast<int>(mazes[i][0].length()),
            static_cast<int>(mazes[i].size()),
            mazes[i]);

        Ms duration = Clock::now() - start;
        total_duration += duration;

        cout << std::fixed << std::setprecision(3)
             << (CONSOLE_LANGUAGE == "FR" ? "Temps de Résolution: " : "Solve Time: ")
             << duration.count() << "ms"
             << "\n";

        correction(
            static_cast<int>(mazes[i][0].length()),
            static_cast<int>(mazes[i].size()),
            mazes[i],
            answer);
    }

    cout << std::fixed << std::setprecision(3)
         << '\n'
         << (CONSOLE_LANGUAGE == "FR" ? "Temps de Résolution Totale: " : "Total Solve Time: ")
         << total_duration.count() << "ms"
         << "\n\n";
}
