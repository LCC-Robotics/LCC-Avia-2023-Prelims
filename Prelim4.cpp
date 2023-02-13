#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// Requires C++17 or higher

// note: code does not work – takes too long to run

template <class T>
using Matrix = std::vector<std::vector<T>>;

enum Action : char {
    FLIP = 'F',
    GRAB = 'G',
    TOWER = 'T',
};

enum Move : char {
    RIGHT = 'R',
    LEFT = 'L',
    DOWN = 'D',
    UP = 'U',
    MOVE_NONE = ' '
};

enum Token : char {
    STARTING_POS = '&',
    YELLOW_PIECE = 'J',
    BLUE_PIECE = 'B',
    ROBOT = '#',
    TOKEN_NONE = ' '
};

struct Coord {
    int32_t row;
    int32_t col;

    constexpr explicit Coord(int32_t row = 0, int32_t col = 0)
        : row { row }
        , col { col }
    {
    }

    constexpr Coord move(Move move)
    {
        Coord new_coord = *this;
        switch (move) {
        case RIGHT: new_coord.col++; break;
        case LEFT: new_coord.col--; break;
        case DOWN: new_coord.row++; break;
        case UP: new_coord.row--; break;
        default: break;
        }
        return new_coord;
    }
    inline void print() const
    {
        std::cout << '(' << row << ',' << col << ')';
    }

    friend inline bool operator==(Coord a, Coord b)
    {
        return a.row == b.row && a.col == b.col;
    }
};

class Avia {
public:
    struct Robot {
        Coord pos;

        const int move_cost;
        const int flip_cost;
        const int grab_cost;
        std::array<int, 8> tower_cost {};

        unsigned int pieces = 0;

        Robot(Coord pos, int move_speed, int flip_speed, int grab_speed, float tower_speed, unsigned int pieces = 0)
            : pos { pos }
            , move_cost { 24 / move_speed }
            , flip_cost { flip_speed }
            , grab_cost { grab_speed }
            , pieces { pieces }
        {
            for (size_t t = 0; t < TOWER_HEIGHTS.size(); ++t) {
                tower_cost[t] = static_cast<int>(std::ceil(TOWER_HEIGHTS[t] / tower_speed));
            }
        };
    };

    explicit Avia(Robot robot, Matrix<bool> is_piece, Matrix<bool> is_blue_piece, Matrix<bool> is_obstacle,
        std::array<int, 3> tower_levels, long time, long initial_score = 0)
        : m_robot { robot }
        , m_time { time }
        , m_score { initial_score }
        , m_is_piece { std::move(is_piece) }
        , m_is_blue_piece { std::move(is_blue_piece) }
        , m_is_obstacle { std::move(is_obstacle) }
        , m_tower_levels { tower_levels }
    {
        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                if (m_is_blue_piece[r][c])
                    m_score += SCORES[0];
            }
        }
    };
    ~Avia() = default;
    Avia(const Avia&) = delete;

    void move(const std::string& actions)
    {
        for (const char& action : actions) {
            move(static_cast<Action>(action));
        }
    }

    void move(char action)
    {
        switch (action) {
        case RIGHT:
            m_robot.pos.col++;
            m_time -= m_robot.move_cost;
            return;
        case LEFT:
            m_robot.pos.col--;
            m_time -= m_robot.move_cost;
            return;
        case DOWN:
            m_robot.pos.row++;
            m_time -= m_robot.move_cost;
            return;
        case UP:
            m_robot.pos.row--;
            m_time -= m_robot.move_cost;
            return;
        case FLIP:
            if (!m_is_piece[m_robot.pos.row][m_robot.pos.col])
                throw std::runtime_error("Piece does not exist at (" + std::to_string(m_robot.pos.row) + ", " + std::to_string(m_robot.pos.col) + ")");

            m_is_blue_piece[m_robot.pos.row][m_robot.pos.col] = !m_is_blue_piece[m_robot.pos.row][m_robot.pos.col];

            m_time -= m_robot.flip_cost;
            m_score += SCORES[0];
            return;
        case GRAB:
            if (!m_is_piece[m_robot.pos.row][m_robot.pos.col])
                throw std::runtime_error("Piece does not exist at (" + std::to_string(m_robot.pos.row) + ", " + std::to_string(m_robot.pos.col) + ")");
            else if (!m_is_blue_piece[m_robot.pos.row][m_robot.pos.col]) // for simplicity, do not allow picking pieces that are not already blue
                throw std::runtime_error("Piece at (" + std::to_string(m_robot.pos.row) + ", " + std::to_string(m_robot.pos.col) + ") is not blue");

            m_is_piece[m_robot.pos.row][m_robot.pos.col] = false;
            m_is_blue_piece[m_robot.pos.row][m_robot.pos.col] = false;

            m_robot.pieces++;
            m_time -= m_robot.grab_cost;
            m_score -= SCORES[0];
            return;
        case TOWER: {
            if (m_robot.pieces == 0)
                throw std::runtime_error("No pieces to add to Tower");

            if (!is_tower(m_robot.pos))
                throw std::runtime_error("Tower does not exist at (" + std::to_string(m_robot.pos.row) + ", " + std::to_string(m_robot.pos.col) + ")");

            const size_t tower_idx = get_tower_index_by_col(m_robot.pos.col);
            const int new_height = ++m_tower_levels[tower_idx];

            m_time -= m_robot.tower_cost[new_height];
            m_score += SCORES[new_height];
            return;
        }
        default: return;
        }
    }

    void undo(const std::string& actions)
    {
        for (auto a = actions.rbegin(); a < actions.rend(); ++a) {
            undo(*a);
        }
    }

    void undo(char action)
    {
        switch (action) {
        case RIGHT:
            m_robot.pos.col--;
            m_time += m_robot.move_cost;
            return;
        case LEFT:
            m_robot.pos.col++;
            m_time += m_robot.move_cost;
            return;
        case DOWN:
            m_robot.pos.row--;
            m_time += m_robot.move_cost;
            return;
        case UP:
            m_robot.pos.row++;
            m_time += m_robot.move_cost;
            return;
        case FLIP:
            if (!m_is_piece[m_robot.pos.row][m_robot.pos.col])
                throw std::runtime_error("Piece does not exist at (" + std::to_string(m_robot.pos.row) + ", " + std::to_string(m_robot.pos.col) + ")");

            m_is_blue_piece[m_robot.pos.row][m_robot.pos.col] = !m_is_blue_piece[m_robot.pos.row][m_robot.pos.col];

            m_time += m_robot.flip_cost;
            m_score -= SCORES[0];
            return;
        case GRAB: {
            if (m_robot.pieces == 0)
                throw std::runtime_error("Cannot undo Grab");

            m_is_piece[m_robot.pos.row][m_robot.pos.col] = true;
            m_is_blue_piece[m_robot.pos.row][m_robot.pos.col] = true;
            m_time += m_robot.grab_cost;
            m_score += SCORES[0];
            return;
        }
        case TOWER: {
            if (is_tower(m_robot.pos))
                throw std::runtime_error("Tower does not exist at (" + std::to_string(m_robot.pos.row) + ", " + std::to_string(m_robot.pos.col) + ")");

            const size_t tower_idx = get_tower_index_by_col(m_robot.pos.col);
            const int& curr_height = m_tower_levels[tower_idx];

            m_time += m_robot.tower_cost[curr_height];
            m_score -= SCORES[curr_height];
            m_tower_levels[tower_idx]--;
            return;
        }
        default: return;
        }
    }

    inline std::array<bool, 3> can_tower() const
    {
        return {
            m_tower_levels[0] < FULL_TOWER[0],
            m_tower_levels[1] < FULL_TOWER[1],
            m_tower_levels[2] < FULL_TOWER[2]
        };
    }

    void print_field() const
    {
        std::vector<std::string> field(ROWS, std::string(COLS, ' '));

        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                if (m_is_piece[r][c]) {
                    field[r][c] = m_is_blue_piece[r][c] ? Token::BLUE_PIECE : Token::YELLOW_PIECE;
                } else if (m_is_obstacle[r][c]) {
                    field[r][c] = Token::ROBOT;
                }
            }
        }

        for (size_t t = 0; t < TOWERS.size(); ++t) {
            const auto& tower = TOWERS[t];
            field[tower.row][tower.col] = m_tower_levels[t] + '0';
        }

        field[m_robot.pos.row][m_robot.pos.col] = Token::STARTING_POS;

        std::cout << "\n+-----------------+\n";

        for (const auto& line : field) {
            std::cout << '|' << line << "|\n";
        }

        std::cout << "+-----------------+\n";
    }

    void print_move(const std::string& move) const
    {
        std::vector<std::string> field(ROWS, std::string(COLS, ' '));

        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                if (m_is_piece[r][c]) {
                    field[r][c] = m_is_blue_piece[r][c] ? Token::BLUE_PIECE : Token::YELLOW_PIECE;
                } else if (m_is_obstacle[r][c]) {
                    field[r][c] = Token::ROBOT;
                }
            }
        }

        for (size_t t = 0; t < TOWERS.size(); ++t) {
            const auto& tower = TOWERS[t];
            field[tower.row][tower.col] = m_tower_levels[t] + '0';
        }

        field[m_robot.pos.row][m_robot.pos.col] = Token::STARTING_POS;

        Coord pos = m_robot.pos;

        for (const char& c : move) {
            pos = pos.move(static_cast<Move>(c));

            field[pos.row][pos.col] = '*';
        }

        std::cout << "\n+-----------------+\n";

        for (const auto& line : field) {
            std::cout << '|' << line << "|\n";
        }

        std::cout << "+-----------------+\n";
    }

    void print_state() const
    {
        print_field();
        std::cout << "Time left:" << m_time << 's' << '\n';
        std::cout << "Score:" << m_score << '\n';
    }

public:
    static constexpr int ROWS = 11;
    static constexpr int COLS = 17;

    static constexpr std::array<float, 8> TOWER_HEIGHTS { 0.0, 1.5, 6.0, 14.5, 25.5, 39.0, 56, 73.5 };
    static constexpr std::array<int, 8> SCORES { 35, 10, 20, 40, 80, 160, 320, 640 };

    static constexpr std::array<Action, 3> ACTIONS { FLIP, GRAB, TOWER };
    static constexpr std::array<Move, 4> MOVES { RIGHT, LEFT, UP, DOWN };

    static constexpr std::array<int, 3> FULL_TOWER { 6, 7, 6 };

    static constexpr Coord TOWER_LEFT { 5, 4 };
    static constexpr Coord TOWER_MIDDLE { 5, 8 };
    static constexpr Coord TOWER_RIGHT { 5, 12 };
    static constexpr std::array<Coord, 3> TOWERS { TOWER_LEFT, TOWER_MIDDLE, TOWER_RIGHT };

    static constexpr inline bool is_tower(Coord c)
    {
        return c.row == 5 && (c.col == 4 || c.col == 8 || c.col == 12);
    }

    static constexpr inline bool is_middle_tower(Coord c)
    {
        return c.row == 5 && c.col == 8;
    }

    static constexpr inline int get_tower_index_by_col(int tower_col)
    {
        switch (tower_col) {
        case 4: return 1;
        case 8: return 2;
        case 12: return 3;
        default: throw std::runtime_error("Not tower index");
        }
    }

public:
    Robot m_robot;
    long m_time;
    long m_score;

    Matrix<bool> m_is_piece;
    Matrix<bool> m_is_blue_piece;
    Matrix<bool> m_is_obstacle;
    std::array<int, 3> m_tower_levels;
};

inline Move get_opposite_move(Move move)
{
    switch (move) {
    case UP: return DOWN;
    case DOWN: return UP;
    case LEFT: return RIGHT;
    case RIGHT: return LEFT;
    default: return MOVE_NONE;
    }
}

using route_generator_t = std::function<const std::string&(Coord from, Coord to)>;

route_generator_t get_partial_route_generator(Avia& game, std::vector<Coord>& nodes)
{
    const size_t nbr_nodes = nodes.size();
    std::vector<std::vector<std::string>> partial_routes(nbr_nodes, std::vector<std::string>(nbr_nodes));

    for (size_t i = 0; i < nodes.size(); ++i) {
        const Coord start_node = nodes[i];

        Matrix<unsigned int> costs(Avia::ROWS, std::vector<unsigned int>(Avia::COLS, UINT_MAX));
        Matrix<Move> directions(Avia::ROWS, std::vector<Move>(Avia::COLS, MOVE_NONE));

        static const auto compare_coord = [&costs](const Coord lhs, const Coord rhs) -> bool {
            return costs[lhs.row][lhs.col] > costs[rhs.row][rhs.col];
        };

        // prioritize processing squares with lower cost (min priority queue)
        std::priority_queue<Coord, std::vector<Coord>, decltype(compare_coord)> frontier { compare_coord };

        costs[start_node.row][start_node.col] = 0;
        frontier.emplace(start_node);

        while (!frontier.empty()) {
            const Coord curr = frontier.top();
            frontier.pop();

            const unsigned int new_cost = costs[curr.row][curr.col] + 1;

            for (const Move& move : Avia::MOVES) {
                const Coord next = Coord(curr).move(move);

                if (next.row < 0 || next.row >= Avia::ROWS || next.col < 0 || next.col >= Avia::COLS // out of bounds
                    || game.m_is_obstacle[next.row][next.col])
                    continue;

                if (new_cost < costs[next.row][next.col]) {
                    costs[next.row][next.col] = new_cost;
                    directions[next.row][next.col] = move;

                    frontier.push(next);
                }
            }
        }

        for (size_t j = 0; j < nodes.size(); ++j) {
            if (i == j)
                continue;

            const Coord end_node = nodes[j];
            const unsigned int cost = costs[end_node.row][end_node.col];

            if (cost == UINT_MAX)
                continue; // impossible to get to node, skip

            std::string partial_route;
            partial_route.resize(cost); // route will be of length cost

            // reconstruct partial route by backtracking from end point to start point
            Coord pos = end_node;

            for (auto rit = partial_route.rbegin(); rit < partial_route.rend(); ++rit) {
                const Move& dir = directions[pos.row][pos.col];
                *rit = dir;
                pos = pos.move(get_opposite_move(dir)); // go backwards
            }

            partial_routes[i][j] = partial_route; // store partial route
        }
    }

    std::unordered_map<int64_t, size_t> coord_to_idx;
    for (size_t i = 0; i < nodes.size(); i++) {
        auto key = *reinterpret_cast<int64_t*>(&nodes[i]); // evil shit – too lazy to declare std::hash<Coord>
        coord_to_idx.emplace(key, i);
    }

    return [partial_routes, coord_to_idx](Coord from, Coord to) -> std::string {
        auto from_key = *reinterpret_cast<int64_t*>(&from);
        auto to_key = *reinterpret_cast<int64_t*>(&to);
        return partial_routes[coord_to_idx.at(from_key)][coord_to_idx.at(to_key)];
    };
}

std::vector<std::string> generate_piece_moves(Avia& game, const route_generator_t& route_generator, bool can_tower)
{
    std::vector<std::string> moves;

    for (int r = 0; r < Avia::ROWS; ++r) {
        for (int c = 0; c < Avia::COLS; ++c) {
            Coord node { r, c };

            if (game.m_robot.pos == node)
                continue;

            if (game.m_is_piece[r][c]) {
                auto route = route_generator(game.m_robot.pos, node);

                if (!game.m_is_blue_piece[r][c]) { // don't flip blue pieces
                    route.push_back(Action::FLIP);
                    moves.push_back(route);
                }

                if (!can_tower)
                    continue;

                route.push_back(Action::GRAB);
                moves.push_back(route);
            }
        }
    }

    return moves;
}

std::vector<std::string> generate_tower_moves(Avia& game, const route_generator_t& route_generator)
{
    const auto can_tower = game.can_tower();

    std::vector<std::string> moves;

    for (size_t t = 0; t < Avia::TOWERS.size(); ++t) {
        if (!can_tower[t])
            continue;

        const auto tower = Avia::TOWERS[t];

        auto route = route_generator(game.m_robot.pos, tower);
        route.push_back(Action::TOWER);
        moves.push_back(route);
    }

    return moves;
}

inline long parse_time(const std::string& time_string)
{
    int minutes = std::stoi(time_string.substr(0, 1));
    int secs = std::stoi(time_string.substr(2, 2));
    return minutes * 60 + secs;
}

inline std::array<int, 3> get_tower_levels(std::vector<std::string>& field)
{
    return {
        field[Avia::TOWER_LEFT.row][Avia::TOWER_LEFT.col] - '0',
        field[Avia::TOWER_MIDDLE.row][Avia::TOWER_MIDDLE.col] - '0',
        field[Avia::TOWER_RIGHT.row][Avia::TOWER_RIGHT.col] - '0'
    };
}

std::pair<std::string, long> solve_impl(Avia& game, const route_generator_t& route_generator, bool is_tower_move = false, bool can_tower = true, int depth = 0)
{
    depth++;
    std::cout << depth << '\n';

    if (game.m_time <= 0)
        return { "", game.m_score }; // return

    long best_score = 0;
    std::string best_move;

    const auto moves = can_tower && is_tower_move
        ? generate_tower_moves(game, route_generator)
        : generate_piece_moves(game, route_generator, can_tower);

    if (is_tower_move && moves.empty()) {
        can_tower = !can_tower;
    }

    for (const auto& move : moves) {
        try {
            game.move(move);

            auto [next_moves, score] = solve_impl(game, route_generator, is_tower_move, can_tower, depth); // recurse

            if (score > best_score) {
                best_score = game.m_score + score;
                best_move = move + next_moves;
            }

            game.undo(move);
        } catch (std::runtime_error& e) {
            std::cerr << e.what() << '\n';
        }
    }

    if (best_score < 0) {
        return { "", game.m_score }; // noop
    }

    return { std::move(best_move), best_score };
}

std::string solve(std::vector<std::string>& field, const std::string& time_string, const int move_speed, const float tower_speed, const int flip_speed, const int grab_speed)
{
    Coord initial_pos;
    Matrix<bool> is_blue_piece(Avia::ROWS, std::vector<bool>(Avia::COLS, false));
    Matrix<bool> is_piece(Avia::ROWS, std::vector<bool>(Avia::COLS, false));
    Matrix<bool> is_obstacle(Avia::ROWS, std::vector<bool>(Avia::COLS, false));

    std::vector<Coord> important_nodes;

    important_nodes.insert(important_nodes.cend(), Avia::TOWERS.begin(), Avia::TOWERS.end());

    for (int r = 0; r < Avia::ROWS; ++r) {
        for (int c = 0; c < Avia::COLS; ++c) {
            switch (field[r][c]) {
            case Token::STARTING_POS:
                initial_pos.row = r;
                initial_pos.col = c;
                break;
            case Token::BLUE_PIECE:
                is_piece[r][c] = true;
                is_blue_piece[r][c] = true;
                important_nodes.emplace_back(r, c);
                break;
            case Token::YELLOW_PIECE:
                is_piece[r][c] = true;
                important_nodes.emplace_back(r, c);
                break;
            case Token::ROBOT:
                is_obstacle[r][c] = true;
                break;
            }
        }
    }

    important_nodes.push_back(initial_pos);

    Avia game {
        Avia::Robot { initial_pos, move_speed, flip_speed, grab_speed, tower_speed },
        std::move(is_piece),
        std::move(is_blue_piece),
        std::move(is_obstacle),
        get_tower_levels(field),
        parse_time(time_string)
    };

    auto partial_route_generator = get_partial_route_generator(game, important_nodes);

    important_nodes.pop_back(); // remove robot inital position (jank ?)

    return std::get<std::string>(solve_impl(game, partial_route_generator));
}

// ==================================
// CORRECTION
// ==================================
#include <chrono>
#include <stack>

// DO NOT MODIFY AFTER THIS LINE / NE PAS MODIFIER APRES CETTE LIGNE

using namespace std;

const string CONSOLE_LANGUAGE = "ENG";
const bool STOP_TESTS_ON_ERROR = false;

constexpr int WIDTH = 17;
constexpr int HEIGHT = 11;

using robot_t = pair<int, int>;

// std::pair is more suitable
vector<robot_t> getRobots(vector<string> grid)
{
    vector<robot_t> ennemies;
    robot_t your_robot;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (grid[i].at(j) == '#') {
                ennemies.emplace_back(j, i);
            } else if (grid[i].at(j) == '&') {
                your_robot = { j, i };
            }
        }
    }
    ennemies.push_back(your_robot);
    return ennemies;
}

// use references on expensive-to-construct objects
bool isValidPosition(int posX, int posY, const vector<robot_t>& robots)
{
    if (posX < 0 || posX >= WIDTH || posY < 0 || posY >= HEIGHT)
        return false;
    for (int i = 0; i < 3; i++) {
        if (posX == get<0>(robots[i]) && posY == get<1>(robots[i]))
            return false;
    }
    return true;
}

// std::tuple for heterogeous types, std::array for homogenous types
// again, use references
inline array<int, 3> getTowerLevels(const vector<string>& grid)
{
    return {
        stoi(grid[5].substr(4, 1)),
        stoi(grid[5].substr(8, 1)),
        stoi(grid[5].substr(12, 1))
    };
}

// trivially copyable, no need pointers
pair<int, int> getTowerScoreTime(int posX, int posY, float speed, array<int, 3> tower_levels)
{
    vector<pair<int, int>> scoreHeight = {
        { ceil(1.5 / speed), 10 },
        { ceil(6.0 / speed), 20 },
        { ceil(14.5 / speed), 40 },
        { ceil(25.5 / speed), 80 },
        { ceil(39.0 / speed), 160 },
        { ceil(55.0 / speed), 320 },
        { ceil(73.5 / speed), 640 }
    };

    // DRY
    if (posY == 5) {
        if (posX == 4 && tower_levels[0] <= 5) {
            return scoreHeight[tower_levels[0]++];
        } else if (posX == 8 && tower_levels[1] <= 6) {
            return scoreHeight[tower_levels[1]++];
        } else if (posX == 12 && tower_levels[2] <= 5) {
            return scoreHeight[tower_levels[2]++];
        }
    }
    return { 0, 0 };
}

// use references
bool checkAnswer(const string& result, vector<string> grid, const string& time_string, int move_speed, float tower_speed, int flip_speed, int grab_time)
{
    bool valid = true;
    const bool fr = CONSOLE_LANGUAGE == "FR";
    const size_t nbr_actions = result.size();

    const int move_time = 24 / move_speed; // 2' = 24"

    const vector<pair<int, int>> robots = getRobots(grid);
    int time = stoi(time_string.substr(0, 1)) * 60 + stoi(time_string.substr(2, 2));
    auto [posX, posY] = robots[3];
    auto tower_levels = getTowerLevels(grid);
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

    for (size_t i = 0; i < nbr_actions; i++) {
        tried_flip = false;
        string action;
        char tile;
        tuple<int, int> tower_score;
        switch (result[i]) {
        case UP:
            action = "Up";
            posY--;
            time -= move_time;
            break;
        case DOWN:
            action = "Down";
            posY++;
            time -= move_time;
            break;
        case LEFT:
            action = "Left";
            posX--;
            time -= move_time;
            break;
        case RIGHT:
            action = "Right";
            posX++;
            time -= move_time;
            break;

        case FLIP:
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

        case GRAB:
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

        case TOWER:
            action = "Tower";
            if (posY != 5 || !(posX == 4 || posX == 8 || posX == 12)) {
                action = fr ? "Vous avez essayer de mettre une piece alors que vous n'etes pas a une tour" : "You have tried to put a game piece but you are not at a tower";
                break;
            }
            tower_score = getTowerScoreTime(posX, posY, tower_speed, tower_levels);
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

        // validity checks
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
        if (!valid)
            break;

        cout << "Pos: (" << posY << "," << posX << ") time: " << time << "s score: " << score << " action: " << action << endl;
    }

    // std::endl prints \n and flushes
    if (!valid)
        cout << "\nSTOP\n"
             << text << '\n'
             << endl;
    else {
        text = fr ? "Voici le temps restant et le score total: " : "Here is the time left and the total score: ";
        cout << '\n'
             << text << time << "s & " << score << " points\n"
             << endl;
    }
    return valid;
}

int main()
{
    const int nbr_cases = 5;

    vector<vector<string>> grids(nbr_cases);

    vector<string> time_left(nbr_cases);
    vector<int> move_speed(nbr_cases);
    vector<float> tower_speed(nbr_cases);
    vector<int> flip_time(nbr_cases);
    vector<int> grab_time(nbr_cases);

    bool passed = true;

    // clang-format off
    grids[0] = { "         BJ      ",
                 "      BJ  JJ J   ",
                 "   J   BBB    J  ",
                 "   &#JJ   B  JJ  ",
                 "   BB    BJ   B  ",
                 " B  2 J 7   5    ",
                 "   J J   # JB J  ",
                 "   #  BJ BJ  J   ",
                 " J   BJ    JBB   ",
                 "     J  J   JJJ  ",
                 "            J    " };
    time_left[0] = "1:31";
    move_speed[0] = 24;
    tower_speed[0] = 20;
    flip_time[0] = 2;
    grab_time[0] = 1;

    // clang-format off
    grids[1] = { "      J JJ  J    ",
        "   J   B J  JB   ",
        "  &B J     J  #J ",
        "  J    J B   J J ",
        "  B J #B  JJ  B  ",
        "  J 2 B 1B  3  JB",
        "J     J   J   J  ",
        "  B J  J #   B J ",
        "  B   JB J B    J",
        "   JJ  J  BJ  B  ",
        "      BJ   JJ    " };
    // clang-format on
    time_left[1] = "3:12";
    move_speed[1] = 12;
    tower_speed[1] = 7.7;
    flip_time[1] = 2;
    grab_time[1] = 2;

    // clang-format off
    grids[2] = { "     B   J J     ",
                 " B  BJJ    J B   ",
                 "   BJ#JJ  JB  B  ",
                 "   J  JJB  JB    ",
                 "     J     J  B  ",
                 " J  1  B0 JJ0 J  ",
                 "   BJ  J    J #  ",
                 "  JB  JJ JJB  BBB",
                 "   &BBB   JB  #  ",
                 "  J   J  JJ   B  ",
                 "     J BJ  JJ    " };
    // clang-format on
    time_left[2] = "3:46";
    move_speed[2] = 8;
    tower_speed[2] = 11.11;
    flip_time[2] = 3;
    grab_time[2] = 2;

    // clang-format off
    grids[3] = { "    J  B         ",
                 "     JB    JB    ",
                 "   J  J  BJJ     ",
                 " J  B  BJ  BJ    ",
                 "  J J J #   B  J ",
                 "J JB4  J3# &6    ",
                 " J  J   #  J    J",
                 "    B  J    JB   ",
                 "    JBJ   BJ     ",
                 "     J  JJ JB B  ",
                 "       J    J    " };
    // clang-format on
    time_left[3] = "1:58";
    move_speed[3] = 12;
    tower_speed[3] = 8.0;
    flip_time[3] = 1;
    grab_time[3] = 3;

    // clang-format off
    grids[4] = { "    JB    B      ",
                 "     JJ    BB    ",
                 "BJ  BB  JJ  J    ",
                 "   JJJ  BJ    B  ",
                 "     BJ   #JJB   ",
                 "  J 0  #5   1JJ  ",
                 " B   B B#  J  JB ",
                 "   J J J    J    ",
                 "  JB  J  B J&J   ",
                 "      BJBB  J  J ",
                 "     J    BB     " };
    // clang-format on
    time_left[4] = "4:02";
    move_speed[4] = 6;
    tower_speed[4] = 31.3;
    flip_time[4] = 3;
    grab_time[4] = 4;

    array<bool, 4> should_test { true, false, false, false };

    using Clock = chrono::high_resolution_clock;
    using Sec = chrono::duration<float, chrono::seconds::period>;

    Sec total_duration;

    // test cases / cas de test
    for (int i = 0; i < nbr_cases; i++) {
        if (!should_test[i])
            continue;

        auto start = Clock::now();

        std::string result = solve(grids[i], time_left[i], move_speed[i], tower_speed[i], flip_time[i], grab_time[i]);

        Sec duration = Clock::now() - start;
        total_duration += duration;

        cout << '\n'
             << "Test " << (i + 1) << '\n'
             << endl;

        passed &= checkAnswer(result, grids[i], time_left[i], move_speed[i], tower_speed[i], flip_time[i], grab_time[i]);

        cout << std::fixed << std::setprecision(2)
             << (CONSOLE_LANGUAGE == "FR" ? "Temps de Résolution: " : "Solve Time: ")
             << duration.count() << "ms"
             << "\n";

        if (STOP_TESTS_ON_ERROR && !passed)
            break;
    }
    // verification of the profit made / verification du profit fait

    // final output / affichage final
    string output;
    if (passed)
        output = CONSOLE_LANGUAGE == "FR" ? "Tout a l'air bon!\n" : "Everything seems good!\n";
    else
        output = CONSOLE_LANGUAGE == "FR" ? "Il semble y avoir un problème avec certaines sorties.\n" : "There seems to be a problem in the outputs somewhere.\n";

    cout << '\n'
         << output << '\n'
         << std::fixed << std::setprecision(2)
         << '\n'
         << (CONSOLE_LANGUAGE == "FR" ? "Temps de Résolution Totale: " : "Total Solve Time: ")
         << total_duration.count() << "ms"
         << '\n'
         << endl;
}
