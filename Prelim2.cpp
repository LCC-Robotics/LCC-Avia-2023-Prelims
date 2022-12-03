#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
using namespace std;
string CONSOLE_LANGUAGE = "ENG";
// L'anglais est mis par defaut. Si vous voulez un affichage en francais, changez le string de la ligne precedente pour "FR"

bool STOP_TESTS_ON_ERROR = true;
// will stop testing at the first problem in the transactions / permet d'arreter de tester si le programme detecte une mauvaise transaction

vector<vector<string>> solve(const int NBR_CITIES, const vector<string>& CITIES_NAMES, vector<vector<int>> cities_demand_supply, vector<vector<int>> transport_costs)
{
    // the format for each transaction is:
    // transactions[i][0] is the city that buys the product / la ville qui achete
    // transactions[i][1] is the city that sells the product / la ville qui vend
    // transactions[i][2] is the amount of unit sold converted in string format / nombre d'unites vendus en string
    std::vector<std::vector<std::string>> transactions;

    std::vector<int> remaining_buying_power(NBR_CITIES);
    std::vector<int> remaining_supply(NBR_CITIES);
    std::vector<int> profit_table(NBR_CITIES * NBR_CITIES); // flattened
    std::vector<int> sorted_idx(NBR_CITIES * NBR_CITIES); // flattened

    for (int i = 0; i < NBR_CITIES; i++) { // row
        remaining_buying_power[i] = cities_demand_supply[i][0];
        remaining_supply[i] = cities_demand_supply[i][2];

        const int buying_price = cities_demand_supply[i][1];

        for (int j = 0; j < NBR_CITIES; j++) { // col
            const int matrix_idx = i * NBR_CITIES + j;

            if (i == j) {
                profit_table[matrix_idx] = 0;
                continue;
            }

            const int selling_price = cities_demand_supply[j][3];
            const int transport_cost = transport_costs[i][j];

            profit_table[matrix_idx] = buying_price - selling_price - transport_cost;
        }
    }

    std::iota(sorted_idx.begin(), sorted_idx.end(), 0);
    std::sort(sorted_idx.begin(), sorted_idx.end(), [&profit_table](int a, int b) {
        return profit_table[a] > profit_table[b];
    });

    for (const int& idx : sorted_idx) {
        if (profit_table[idx] <= 0)
            break;

        const int buyer = idx / NBR_CITIES; // row
        const int seller = idx % NBR_CITIES; // col

        const int buying_power = remaining_buying_power[buyer];
        const int supply = remaining_supply[seller];

        if (buying_power <= 0 || supply <= 0)
            continue;

        const int transaction_amount = buying_power > supply ? supply : buying_power;

        remaining_buying_power[buyer] -= transaction_amount;
        remaining_supply[seller] -= transaction_amount;

        transactions.push_back({ CITIES_NAMES[buyer], CITIES_NAMES[seller], std::to_string(transaction_amount) });
	}

    return transactions;
}

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

bool checkAnswer(const vector<vector<string>>& TRANSACTIONS, const int NBR_CITIES, const vector<string>& CITIES_NAMES, vector<vector<int>> demand_supply, vector<vector<int>> transport)
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
    vector<vector<string>> CITIES_NAMES(nbr_cases);
    vector<vector<vector<int>>> CITIES_DEMAND_SUPPLY(nbr_cases);
    vector<vector<vector<int>>> TRANSPORT_COST(nbr_cases);
    vector<vector<vector<string>>> result = {};
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
    TRANSPORT_COST[2] = { { 0, 1, 3, 2, 3, 2, 1 }, { 1, 0, 1, 2, 2, 3, 3 }, { 1, 1, 0, 1, 1, 3, 3 }, { 1, 3, 2, 0, 2, 2, 3 }, { 2, 1, 3, 2, 0, 1, 2 }, { 2, 2, 2, 1, 2, 0, 1 }, { 2, 2, 2, 2, 3, 2, 3, 0 } };

    CITIES_NAMES[3] = { "Londres", "Birmingham", "Liverpool", "Lille", "Paris", "Marseille", "Tours", "Fribourg", "Amsterdam", "Berlin" };
    CITIES_DEMAND_SUPPLY[3] = { { 90, 103, 160, 99 }, { 170, 104, 220, 98 }, { 230, 103, 250, 98 }, { 90, 103, 60, 99 }, { 180, 104, 160, 100 }, { 140, 104, 200, 97 }, { 110, 103, 110, 99 }, { 10, 103, 240, 100 }, { 190, 104, 220, 100 }, { 140, 104, 50, 97 } };
    TRANSPORT_COST[3] = { { 0, 2, 3, 3, 2, 1, 1, 3, 3, 1 }, { 1, 0, 2, 1, 3, 1, 2, 1, 3, 1 }, { 1, 3, 0, 3, 1, 1, 2, 1, 3, 1 }, { 3, 2, 2, 0, 3, 2, 2, 2, 1, 1 }, { 2, 2, 1, 3, 0, 1, 1, 2, 1, 1 }, { 3, 3, 2, 2, 1, 0, 3, 3, 1, 3 }, { 1, 1, 1, 1, 3, 3, 0, 2, 1, 2 }, { 2, 3, 1, 2, 1, 1, 3, 0, 3, 1 }, { 1, 1, 2, 2, 3, 3, 2, 1, 0, 2 }, { 2, 2, 2, 1, 3, 1, 3, 1, 3, 0 } };

    CITIES_NAMES[4] = { "New York", "Boston", "Chicago", "Atlanta", "Washington", "Portland", "Austin", "Houston", "Albuquerque", "Sacramento", "San Francisco", "Los Angeles" };
    CITIES_DEMAND_SUPPLY[4] = { { 50, 104, 130, 97 }, { 220, 104, 160, 98 }, { 170, 104, 140, 98 }, { 210, 102, 120, 97 }, { 210, 103, 60, 97 }, { 120, 102, 120, 97 }, { 70, 101, 60, 100 }, { 110, 104, 140, 97 }, { 130, 104, 90, 100 }, { 170, 103, 80, 98 }, { 70, 101, 120, 97 }, { 100, 103, 250, 98 } };
    TRANSPORT_COST[4] = { { 0, 2, 1, 2, 3, 3, 1, 1, 1, 3, 3, 3 }, { 1, 0, 3, 2, 3, 2, 2, 2, 1, 1, 1, 3 }, { 1, 2, 0, 1, 2, 2, 1, 3, 3, 2, 2, 1 }, { 3, 1, 1, 0, 1, 3, 1, 3, 3, 3, 1, 1 }, { 1, 3, 3, 1, 0, 1, 3, 1, 3, 2, 1, 3 }, { 3, 2, 1, 2, 3, 0, 1, 2, 1, 2, 3, 1 }, { 3, 2, 1, 2, 2, 2, 0, 3, 3, 3, 2, 3 }, { 1, 2, 1, 3, 2, 3, 1, 0, 3, 3, 3, 2 }, { 2, 2, 1, 2, 1, 3, 3, 2, 0, 2, 2, 3 }, { 1, 3, 1, 3, 1, 2, 2, 2, 3, 0, 1, 1 }, { 3, 1, 1, 3, 2, 3, 2, 3, 1, 1, 0, 2 }, { 3, 1, 1, 1, 1, 3, 2, 2, 1, 2, 1, 0 } };

    // calls your code / appel de votre code
    for (int i = 0; i < nbr_cases; i++) {
        result.push_back(solve(NBR_CITIES[i], CITIES_NAMES[i], CITIES_DEMAND_SUPPLY[i], TRANSPORT_COST[i]));
    }
    // verification of the profit made / verification du profit fait
    for (int i = 0; i < nbr_cases; i++) {
        cout << endl
             << "Test " << (i + 1) << endl
             << endl;
        passed &= checkAnswer(result[i], NBR_CITIES[i], CITIES_NAMES[i], CITIES_DEMAND_SUPPLY[i], TRANSPORT_COST[i]);
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
