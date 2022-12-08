#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

template <class T = float>
using Matrix = std::vector<std::vector<T>>;

void printTableau(const Matrix<float>& tableau, const int nbr_objective, const int nbr_slack, const int precision = 4, const int cell_width = 5)
{
    std::ios old_state(nullptr);
    old_state.copyfmt(std::cout);

    const std::string hor_line((nbr_objective + nbr_slack + 2) * cell_width + 1, '-');

    for (int obj_var = 0; obj_var < nbr_objective; ++obj_var) {
        std::cout << std::setw(cell_width) << "x" + std::to_string(obj_var);
    }

    for (int slack_var = 0; slack_var < nbr_slack; ++slack_var) {
        std::cout << std::setw(cell_width) << "s" + std::to_string(slack_var);
    }

    std::cout << std::setw(cell_width) << "z";
    std::cout << std::setw(cell_width) << "b";

    for (auto row = tableau.begin(); row < tableau.end(); ++row) {
        if (row == tableau.end() - 1)
            std::cout << '\n'
                      << hor_line;

        std::cout << '\n';
        for (auto cell = row->begin(); cell < row->end(); ++cell) {
            if (cell == row->end() - 1)
                std::cout << "|";
            std::cout << std::setw(cell_width) << std::setprecision(precision) << *cell;
        }
    }
    std::cout << std::endl;
    std::cout.copyfmt(old_state);
}

Matrix<std::string> solve(const int NBR_CITIES, const std::vector<std::string> CITIES_NAMES, const Matrix<int> cities_demand_supply, const Matrix<int> transport_costs)
{
    Matrix<std::string> transactions;

    constexpr auto compare_float = [](float a, float b, float epsilon = 1.0e-8f) {
        return std::fabsf(a - b) < epsilon;
    };

    /*
    https://www.hec.ca/en/cams/help/topics/The_steps_of_the_simplex_algorithm.pdf
    https://www.youtube.com/watch?v=iwDiG2mR6FM
    https://www.youtube.com/watch?v=pVWsXZh81IU
    */

    const int NBR_CONSTRAINTS = NBR_CITIES * 2;
    const int NBR_DECISION_VARS = NBR_CITIES * NBR_CITIES;
    const int& NBR_SLACK_VARS = NBR_CONSTRAINTS;
    const int NBR_TOTAL_VARS = NBR_DECISION_VARS + NBR_SLACK_VARS + 1; // + 1 because of z variable

    const int NBR_TABLEAU_ROWS = NBR_CONSTRAINTS + 1; // nbr of constraints + objective row
    const int NBR_TABLEAU_COLS = NBR_TOTAL_VARS + 1; // nbr of variables + rhs row

    const int& SLACK_SECTION_BEGIN = NBR_DECISION_VARS; // idex at which slack section begins
    const int& RHS_COLUMN = NBR_TABLEAU_COLS - 1; // last column
    const int& OBJECTIVE_ROW = NBR_TABLEAU_ROWS - 1; // last row

    Matrix<float> tableau(NBR_TABLEAU_ROWS, std::vector<float>(NBR_TABLEAU_COLS, 0));
    std::vector<int> basic(NBR_CONSTRAINTS); // keep track of basic variables, i is row, basic[i] is index of variable

    /*
    Example Tableau

       variable index   0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24    25

                                                                                                SLACK_SECTION_BEGIN
                                                                                                       |
                                                    Decision Variables                                 v         Slack Variables               RHS_COLUMN
                      ┌─────────────────────────────────────────────────────────────────────────────┐ ┌─────────────────────────────────────┐       |
                      │                                                                             │ │                                     │       v
                       x0   x1   x2   x3   x4   x5   x6   x7   x8   x9  x10  x11  x12  x13  x14  x15   s0   s1   s2   s3   s4   s5   s6   s7    z│    b
                    ─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┼─────
                ┌─      1    0    1    0    0   -1    0   -1    1    0    1    0    1    0    1    0    1    0    1    1    0   -1    0   -1    0│   30
                │       0    0    0    0    1    1    1    1    0    0    0    0    0    0    0    0    0    1    0    0    0    0    0    0    0│   90
                │      -1    0   -1   -1    0    1    0    0    0    1    0    0   -1    0   -1   -1   -1    0    0   -1    0    1    0    0    0│   30
    Constraints │       0    0    0    1    0    0    0    1    0    0    0    1    0    0    0    1    0    0    0    0    0    0    0    1    0│  140
                │       0    0   -1    0    0    0   -1    0    0    0   -1    0    0    0   -1    0   -1   -1   -1   -1    1    1    0    1    0│   40
                │       0    0    0    0    0    0    0    0    0    0    0    0    1    1    1    1    0    0    0    1    0    0    0    0    0│   70
                │       0    0    1    0    0    0    1    0    0    0    1    0    0    0    1    0    0    0    0    0    0    0    1    0    0│  120
                └─      1    1    1    1    0    0    0    0    0    0    0    0    0    0    0    0    1    0    0    0    0    0    0    0    0│  150
                    ─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┼─────
    OBJECTIVE_ROW ->    2    0    2    0    0    3    3    0    0    0    4    0    0    0    2    4    2    1    4    3    0    2    0    1    1│ 2040  <-- Max Profit


    * Decision Variables: Represents every possible permutation (total permutations = NBR_CITIES^2)
        -> index = buyer * NBR_CITIES + seller
        -> buyer = floor(index / NBR_CITIES)
        -> seller = index % NBR_CITIES
    * Slack Variables: Convert inequality to equality
    * RHS Variables: The values of basic variables
    * Constraints: Represents the limits on buying and selling amounts (total constraints = NBR_CITIES*2)
        -> each city has an equation for max buying amount and max selling amount
    * Objective Function: function to maximize
        -> the coefficients of the decision variables is the net profit per unit
    */

    /*
    Format of cities_demand_supply:
    [0]: buying quantity
    [1]: buying price
    [2]: selling quantity
    [3]: selling price
    */

    // Optimized for loop which sets up and populates the matrices and vectors above
    for (int buyer_idx = 0; buyer_idx < NBR_CITIES; ++buyer_idx) { // row
        const int& buying_price = cities_demand_supply[buyer_idx][1];

        for (int seller_idx = 0; seller_idx < NBR_CITIES; ++seller_idx) { // col
            int idx = buyer_idx * NBR_CITIES + seller_idx;

            // set up constraints coefficients
            // for each city, a equation can be set up for limiting buying quantity and one for limiting selling quantity
            tableau[buyer_idx][idx] = 1.0; // equation for limited buying quantity
            if (seller_idx * NBR_CITIES + buyer_idx < NBR_DECISION_VARS) {
                tableau[buyer_idx + NBR_CITIES][seller_idx * NBR_CITIES + buyer_idx] = 1.0; // equation for limited selling quantity
            }

            // buyer and seller cannot be the same city.
            if (buyer_idx == seller_idx)
                continue;

            const int& selling_price = cities_demand_supply[seller_idx][3];
            const int& transport_cost = transport_costs[seller_idx][buyer_idx]; // transport_costs[from][to]

            tableau[OBJECTIVE_ROW][idx] = -(buying_price - selling_price - transport_cost); // standard form
        }

        // Since NBR_CONSRTAINTS = NBR_CITIES * 2, we initialize 2 rhs and 2 slack variables per iteration of for loop
        // RHS of constraints
        tableau[buyer_idx][RHS_COLUMN] = cities_demand_supply[buyer_idx][0]; // buying quantity
        tableau[NBR_CITIES + buyer_idx][RHS_COLUMN] = cities_demand_supply[buyer_idx][2]; // selling quantity

        // Slack variables - initially set them as the basic variables
        tableau[buyer_idx][SLACK_SECTION_BEGIN + buyer_idx] = 1.0;
        tableau[NBR_CITIES + buyer_idx][SLACK_SECTION_BEGIN + NBR_CITIES + buyer_idx] = 1.0;
    }

    std::iota(basic.begin(), basic.end(), SLACK_SECTION_BEGIN); // set slack variables as basic

    tableau[OBJECTIVE_ROW][RHS_COLUMN - 1] = 1.0; // set z

    // Simplex time!
    Matrix<float> new_tableau(tableau); // create copy

    // int iterations = 0;
    while (true) {
        // std::cout << "Iteration: " << iterations++ << '\n';
        // printTableau(tableau, NBR_OBJECTIVE_VARS, NBR_SLACK_VARS);

        // Step 1: Identify pivot using Bland's rule (to avoid cycles goddamnit)
        int entering, leaving; // col, row

        // pivot_j is column with the smallest negative value
        float min_value = std::numeric_limits<float>::infinity();
        for (int j = 0; j < NBR_TOTAL_VARS; ++j) {
            auto value = tableau[OBJECTIVE_ROW][j];
            if (value < min_value
                && std::find(basic.begin(), basic.end(), j) == basic.end() // Bland's rule: Pivot only on non-basic columns
            ) {
                min_value = value;
                entering = j;
            }
        }

        if (min_value >= 0)
            break; // optimal solution is found

        // pivot_i is the row where the rhs[i] divided by constraints[i][pivot_j] is the smallest
        float min_ratio = std::numeric_limits<float>::infinity();
        for (int i = 0; i < NBR_CONSTRAINTS; ++i) {
            if (compare_float(tableau[i][entering], 0.0))
                continue;

            float ratio = tableau[i][RHS_COLUMN] / tableau[i][entering];

            if (
                (ratio < min_ratio && tableau[i][entering] > 0) // min ratio test
                || (ratio == min_ratio && i < leaving) // Bland's rule: When there is a tie (degeneracy), choose the variable with a lower index
            ) {
                min_ratio = ratio;
                leaving = i;
            }
        }

        // Step 2: divide pivot row by pivot to make coefficient at pivot 1
        const float pivot_value = tableau[leaving][entering];
        for (int j = 0; j < NBR_TABLEAU_COLS; ++j) {
            new_tableau[leaving][j] /= pivot_value; 
        }

        // Step 3: Make it the rest of the values on the pivot row are zero to make pivot column basic
        for (int i = 0; i < NBR_TABLEAU_ROWS; ++i) {
            if (i == leaving) // skip pivot row
                continue;
            for (int j = 0; j < NBR_TABLEAU_COLS; ++j) {
                // Gaussian Elimination: 
                // new value = negative old value on pivot col * new value on pivot row + old value
                new_tableau[i][j] -= tableau[i][entering] * new_tableau[leaving][j]; 
            }
        }

        tableau = new_tableau;
        basic[leaving] = entering; // update basic vars
    }

    // printTableau(tableau, NBR_OBJECTIVE_VARS, NBR_SLACK_VARS);

    /*
    Populate transactions with the results
    Format:
    transactions[i][0] is the city that buys the product / la ville qui achete
    transactions[i][1] is the city that sells the product / la ville qui vend
    transactions[i][2] is the amount of unit sold converted in string format / nombre d'unites vendus en string
    */

    for (std::size_t i = 0; i < basic.size(); ++i) {
        int j = basic[i];

        if (j < NBR_DECISION_VARS) {
            const int buyer = j / NBR_CITIES;
            const int seller = j % NBR_CITIES;
            const int amount = tableau[i][RHS_COLUMN] / tableau[i][j];

            if (amount == 0)
                continue; // skip degenerate basic variable

            transactions.emplace_back(
                std::initializer_list<std::string> { CITIES_NAMES[buyer], CITIES_NAMES[seller], std::to_string(amount) });
        }
    }

    return transactions;
}

using namespace std;
// L'anglais est mis par defaut. Si vous voulez un affichage en francais, changez le string de la ligne precedente pour "FR"
string CONSOLE_LANGUAGE = "ENG";
// will stop testing at the first problem in the transactions / permet d'arreter de tester si le programme detecte une mauvaise transaction
bool STOP_TESTS_ON_ERROR = false;

// DO NOT MODIFY AFTER THIS LINE / NE PAS MODIFIER APRES CETTE LIGNE
void printTransaction(const int idx, const vector<vector<string>>& TRANSACTIONS)
{
    cout << "[\"" << TRANSACTIONS[idx][0] << "\", \"" << TRANSACTIONS[idx][1] << "\", \"" << TRANSACTIONS[idx][2] << "\"]";
}

int getCityIdx(string cityName, const int NBR_CITIES, const vector<string>& CITIES_NAMES)
{
    for (int i = 0; i < NBR_CITIES; i++) {
        if (cityName.compare(CITIES_NAMES[i]) == 0)
            return i;
    }
    return -1;
}

bool checkAnswer(const vector<vector<string>>& TRANSACTIONS, const int NBR_CITIES, const vector<string>& CITIES_NAMES, vector<vector<int>> demand_supply, const vector<vector<int>>& transport)
{
    bool fr = CONSOLE_LANGUAGE.compare("FR") == 0;
    string output;
    int nbr_tr = TRANSACTIONS.size();
    int profit = 0;

    // check if there's at least one transaction / regarde s'il y a au moins une transaction
    if (nbr_tr == 0) {
        output = fr ? "Vous n'avez fait aucune transaction" : "You have made no transactions";
        cout << output << endl;
        return false;
    }
    // check if the format of the transactions / regarde le formatage des transactions
    for (vector<string> transaction : TRANSACTIONS) {
        if (transaction.size() != 3) {
            output = fr ? "Une transaction ne suit pas le bon format" : "A transaction doesn't have the right format";
            cout << output << endl;
            return false;
        }
    }

    // quick display of your output / petit affichage de votre sortie
    output = fr ? "Votre Sortie:" : "Your Output:";
    cout << output << endl;
    for (int i = 0; i < nbr_tr; i++) {
        printTransaction(i, TRANSACTIONS);
        cout << endl;
    }
    cout << endl;

    for (int i = 0; i < nbr_tr; i++) {
        int amount = 0;
        try {
            amount = stoi(TRANSACTIONS[i][2]);
            if (amount < 0) {
                // check for positive amount / regarde pour quantites positives
                output = fr ? "La quantite d'unites doit etre positive" : "The quantity needs to be positive";
                cout << output << endl;
                return false;
            }
        } catch (...) {
            // check if quantity is a number / regarde si la quantite est un nombre
            output = fr ? "La quantite d'unites n'est pas un entier" : "The quantity is not an integer";
            cout << output << endl;
            return false;
        }

        // get idx of the receiver and the sender / trouver l'indice de l'acheteur et du vendeur
        int receiverIdx = getCityIdx(TRANSACTIONS[i][0], NBR_CITIES, CITIES_NAMES);
        int senderIdx = getCityIdx(TRANSACTIONS[i][1], NBR_CITIES, CITIES_NAMES);

        // check if a city is invalid / regarde si une ville est invalide
        if (receiverIdx == -1 || senderIdx == -1) {
            cout << "Transaction ";
            printTransaction(i, TRANSACTIONS);
            output = fr ? " contient une ville non-existante" : " has an invalid city name";
            cout << output << endl;
            return false;
        }
        // check if transaction is with itself / regarde si la transaction contient seulement une ville
        if (receiverIdx == senderIdx) {
            cout << "Transaction ";
            printTransaction(i, TRANSACTIONS);
            output = fr ? " contient deux fois la meme ville" : " has the same city twice";
            cout << output << endl;
            return false;
        }

        // substract the product from the transaction / soustraire les produits de la transaction
        demand_supply[receiverIdx][0] -= amount;
        demand_supply[senderIdx][2] -= amount;

        // check for max capacity of receiver / regarde que la capacite maximale du demandeur est correcte
        if (demand_supply[receiverIdx][0] < 0) {
            cout << "Transaction ";
            printTransaction(i, TRANSACTIONS);
            output = fr ? "\nDépasse la capacité d'achat de la ville de " : "\nExceeds the quantity that can be bought by ";
            cout << output << CITIES_NAMES[receiverIdx] << endl;
            return false;
        }

        // check for max capacity of the sender / regarde pour etre correct avec la capacite maximale
        if (demand_supply[senderIdx][2] < 0) {
            cout << "Transaction ";
            printTransaction(i, TRANSACTIONS);
            output = fr ? "\nDépasse la capacité de vente de la ville de " : "\nExceeds the quantity that can be selled by ";
            cout << output << CITIES_NAMES[senderIdx] << endl;
            return false;
        }

        // profit calculation / calcul du profit
        int transactionProfit = (demand_supply[receiverIdx][1] - demand_supply[senderIdx][3] - transport[senderIdx][receiverIdx]) * amount;

        // transaction validation ouput / sortie de validation de transaction
        cout << "Transaction ";
        printTransaction(i, TRANSACTIONS);
        output = fr ? " autorisée!\nProfit fait par cette transaction: " : " authorized\nProfit made by this transaction: ";
        cout << output << transactionProfit << "$" << endl;
        profit += transactionProfit;
    }

    output = fr ? "Profit total obtenu: " : "Total profit made: ";
    cout << endl
         << output << profit << "$" << endl
         << endl;
    return true;
}

int main()
{
    const int nbr_cases = 5;
    const bool test[5] = { true, true, true, true, true };
    vector<vector<string>> CITIES_NAMES(nbr_cases);
    vector<vector<vector<int>>> CITIES_DEMAND_SUPPLY(nbr_cases);
    vector<vector<vector<int>>> TRANSPORT_COST(nbr_cases);
    bool passed = true;

    // test cases / cas de test
    int NBR_CITIES[nbr_cases] = { 4, 5, 7, 10, 12 };

    CITIES_NAMES[0] = { "Pékin", "Séoul", "Pyongyang", "Kyoto" };
    CITIES_DEMAND_SUPPLY[0] = { { 150, 103, 160, 98 }, { 90, 101, 250, 97 }, { 200, 104, 120, 100 }, { 70, 104, 140, 97 } };
    TRANSPORT_COST[0] = { { 0, 2, 2, 3 }, { 2, 0, 1, 2 }, { 3, 3, 0, 3 }, { 3, 2, 2, 0 } };

    CITIES_NAMES[1] = { "Kuala Lumpur", "Singapour", "Jakarta", "Melbourne", "Auckland" };
    CITIES_DEMAND_SUPPLY[1] = { { 220, 101, 50, 99 }, { 50, 103, 200, 100 }, { 150, 101, 140, 97 }, { 120, 104, 50, 97 }, { 130, 104, 190, 97 } };
    TRANSPORT_COST[1] = { { 0, 1, 2, 1, 2 }, { 2, 0, 1, 2, 2 }, { 1, 3, 0, 2, 2 }, { 1, 1, 3, 0, 3 }, { 1, 1, 1, 3, 0 } };

    CITIES_NAMES[2] = { "Montréal", "Vancouver", "Trois-Rivières", "Winnipeg", "Halifax", "Toronto", "Kuujjuaq" };
    CITIES_DEMAND_SUPPLY[2] = { { 70, 102, 200, 97 }, { 250, 103, 240, 99 }, { 170, 101, 60, 97 }, { 80, 101, 190, 100 }, { 190, 104, 250, 97 }, { 100, 102, 160, 99 }, { 230, 104, 50, 100 } };
    TRANSPORT_COST[2] = { { 0, 1, 3, 2, 3, 2, 1 }, { 1, 0, 1, 2, 2, 3, 3 }, { 1, 1, 0, 1, 1, 3, 3 }, { 1, 3, 2, 0, 2, 2, 3 }, { 2, 1, 3, 2, 0, 1, 2 }, { 2, 2, 2, 1, 2, 0, 1 }, { 2, 2, 2, 2, 3, 2, 0 } };

    CITIES_NAMES[3] = { "Londres", "Birmingham", "Liverpool", "Lille", "Paris", "Marseille", "Tours", "Fribourg", "Amsterdam", "Berlin" };
    CITIES_DEMAND_SUPPLY[3] = { { 90, 103, 160, 99 }, { 170, 104, 220, 98 }, { 230, 103, 250, 98 }, { 90, 103, 60, 99 }, { 180, 104, 160, 100 }, { 140, 104, 200, 97 }, { 110, 103, 110, 99 }, { 10, 103, 240, 100 }, { 190, 104, 220, 100 }, { 140, 104, 50, 97 } };
    TRANSPORT_COST[3] = { { 0, 2, 3, 3, 2, 1, 1, 3, 3, 1 }, { 1, 0, 2, 1, 3, 1, 2, 1, 3, 1 }, { 1, 3, 0, 3, 1, 1, 2, 1, 3, 1 }, { 3, 2, 2, 0, 3, 2, 2, 2, 1, 1 }, { 2, 2, 1, 3, 0, 1, 1, 2, 1, 1 }, { 3, 3, 2, 2, 1, 0, 3, 3, 1, 3 }, { 1, 1, 1, 1, 3, 3, 0, 2, 1, 2 }, { 2, 3, 1, 2, 1, 1, 3, 0, 3, 1 }, { 1, 1, 2, 2, 3, 3, 2, 1, 0, 2 }, { 2, 2, 2, 1, 3, 1, 3, 1, 3, 0 } };

    CITIES_NAMES[4] = { "New York", "Boston", "Chicago", "Atlanta", "Washington", "Portland", "Austin", "Houston", "Albuquerque", "Sacramento", "San Francisco", "Los Angeles" };
    CITIES_DEMAND_SUPPLY[4] = { { 50, 104, 130, 97 }, { 220, 104, 160, 98 }, { 170, 104, 140, 98 }, { 210, 102, 120, 97 }, { 210, 103, 60, 97 }, { 120, 102, 120, 97 }, { 70, 101, 60, 100 }, { 110, 104, 140, 97 }, { 130, 104, 90, 100 }, { 170, 103, 80, 98 }, { 70, 101, 120, 97 }, { 100, 103, 250, 98 } };
    TRANSPORT_COST[4] = { { 0, 2, 1, 2, 3, 3, 1, 1, 1, 3, 3, 3 }, { 1, 0, 3, 2, 3, 2, 2, 2, 1, 1, 1, 3 }, { 1, 2, 0, 1, 2, 2, 1, 3, 3, 2, 2, 1 }, { 3, 1, 1, 0, 1, 3, 1, 3, 3, 3, 1, 1 }, { 1, 3, 3, 1, 0, 1, 3, 1, 3, 2, 1, 3 }, { 3, 2, 1, 2, 3, 0, 1, 2, 1, 2, 3, 1 }, { 3, 2, 1, 2, 2, 2, 0, 3, 3, 3, 2, 3 }, { 1, 2, 1, 3, 2, 3, 1, 0, 3, 3, 3, 2 }, { 2, 2, 1, 2, 1, 3, 3, 2, 0, 2, 2, 3 }, { 1, 3, 1, 3, 1, 2, 2, 2, 3, 0, 1, 1 }, { 3, 1, 1, 3, 2, 3, 2, 3, 1, 1, 0, 2 }, { 3, 1, 1, 1, 1, 3, 2, 2, 1, 2, 1, 0 } };

    // calls your code / appel de votre code
    for (int i = 0; i < nbr_cases; i++) {
        if (!test[i])
            continue;

        auto result = solve(NBR_CITIES[i], CITIES_NAMES[i], CITIES_DEMAND_SUPPLY[i], TRANSPORT_COST[i]);
        cout << endl
             << "Test " << (i + 1) << endl
             << endl;
        passed &= checkAnswer(result, NBR_CITIES[i], CITIES_NAMES[i], CITIES_DEMAND_SUPPLY[i], TRANSPORT_COST[i]);
        if (STOP_TESTS_ON_ERROR && !passed)
            break;
    }

    // final output / affichage final
    string output;
    if (passed)
        output = CONSOLE_LANGUAGE == "FR" ? "Tout a l'air bon!\n" : "Everything seems good!\n";
    else
        output = CONSOLE_LANGUAGE == "FR" ? "Il semble y avoir un problème avec certaines sorties.\n" : "There seems to be a problem in the outputs somewhere.\n";
    cout << endl
         << output;
}
