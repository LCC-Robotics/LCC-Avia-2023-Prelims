#include <array>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
// L'anglais est mis par defaut. Si vous voulez un affichage en francais,
// changez le string de la ligne suivante pour "FR"
string CONSOLE_LANGUAGE = "ENG";
// DO NOT DELETE STATEMENT / NE PAS EFFACER CETTE DÉCLARATION

string solve(string testCase)
{
    const std::array<std::string, 15> nouns = {
        "montreal", "quebec", "toronto", "vancouver", "canada", "julie", "jimmy", "louis",
        "andrae", "francois", "xavier", "elrik", "simon", "jeff", "charles"
    };

    constexpr auto is_punctuation = [](char c) -> bool {
        return (c == '_' || c == '-' || c == '\'' || c == ',' || c == '.' || c == '!' || c == '?');
    };

    constexpr auto is_sentence_delimiter = [](char c) -> bool {
        return (c == '.' || c == '?' || c == '!');
    };

    const auto is_noun = [&](const std::string& s) -> bool {
        return std::any_of(nouns.begin(), nouns.end(),
            [&](const std::string& val) {
                return s.compare(val) == 0; //
            });
    };

    std::string answer;

    testCase[0] = std::toupper(testCase.front()); // capitalize first letter
    std::istringstream iss { testCase }; // string -> string stream

    std::string token;
    std::string prev_token;

    while (std::getline(iss, token, ' ')) { // parse word by word
        char delimiter = ' ';

        if (std::isdigit(token.front())) {
            // Rule 1 - simplify math operations
            std::string temp = "";
            int arg1 = 0, arg2 = 0;
            char op = 0;

            for (const char& c : token) {
                if (std::isdigit(c)) {
                    temp += c;
                } else {
                    op = c;
                    arg1 = std::stoi(temp);
                    temp.clear();
                }
            }

            if (op) {
                arg2 = std::stoi(temp);

                switch (op) {
                case '+':
                    token = std::to_string(arg1 + arg2);
                    break;
                case '-':
                    token = std::to_string(arg1 - arg2);
                    break;
                case '*':
                    token = std::to_string(arg1 * arg2);
                    break;
                case '/':
                    token = std::to_string(arg1 / arg2);
                    break;
                }
            }
        } else if (!std::isupper(token.front())) { // do not need to process words that begin with uppercase
            std::string stripped_word = is_punctuation(token.back())
                ? token.substr(0, token.size() - 1) // if the last letter is punctuation, remove the punctuation
                : token;

            if (
                is_sentence_delimiter(prev_token.back()) // Rule 2 - capitalize if the preceding token ends in punctuation
                || is_noun(stripped_word) // Rule 3 - capitalize if word is in list of nouns
            ) {
                token[0] = std::toupper(token.front());

            } else { 
                int alpha_len = token.size(); // length of string excluding punctuation
                int total_value = 0; // sum of the corresponding numerical value of alphabetical letter

                for (const char& c : token) {
                    // using one loop to do two things so efficient wor
                    if (c >= 'a' && c <= 'z') {
                        total_value += c - 'a' + 1;
                    } else if (is_punctuation(c)) {
                        alpha_len--;
                    }
                }
                
                // Rule 4 - add '_' at midpoint if length excluding punctuation is divisible by 2
                if (alpha_len != 0 && alpha_len % 2 == 0) {
                    const int midpoint = alpha_len / 2;
                    int alpha_count = 0;

                    for (auto it = token.begin(); it < token.end(); it++) {
                        if (!is_punctuation(*it)) {
                            alpha_count++;

                            if (alpha_count == midpoint) {
                                token.insert(it + 1, 1, '_');
                                break;
                            }
                        }
                    }
                }

                // Rule 5 - use tab as delimiter if the sum of the corresponding numerical value
                //          of the letters is divisible by 5
                if (alpha_len != 0 && total_value % 5 == 0) {
                    delimiter = '\t';
                }
            }
        }
        answer += token + delimiter;
        prev_token = token;
    }

    if (answer.back() == ' ') // strip whitespace from end
    {
        answer.resize(answer.size() - 1);
    }

    return answer; 
}

// Do not modify anything beyond this point. / Ne rien modifier après ce point.
// The rest of this code is to facilitate the validation and testing of your
// code. Le reste de ce code sert à faciliter la validation et le test de votre
// code.

// Function to show if your code works / Fonction pour vous montrer si votre
// code fonctionne
void checkAnswers(string* result, string* expected, int nbr_cases)
{
    int nbr_tests_passed = 0;
    bool fr = CONSOLE_LANGUAGE.compare("FR") == 0;
    string output = fr ? "Votre Sortie:\n" : "Your Output:\n";
    cout << output;

    // #Quick print to show all your code's outputs on different lines / Affichage rapide pour vous montrer vos sorties
    for (int i = 0; i < nbr_cases; i++) {
        cout << result[i] << "\n";
    }

    // To tell you if a testcase is successful / Pour vous dire si un test est réussi
    for (int i = 0; i < nbr_cases; i++) {
        output = fr ? "\n\nTest " : "\n\nCase ";
        cout << output;
        cout << i + 1;
        if (result[i].compare(expected[i]) == 0) {
            output = fr ? " réussi!" : " successful!";
            cout << output;
            nbr_tests_passed++;
        } else {
            output = fr ? " non réussi.\nOn s'attendait à: \"" : " unsuccessful.\nExpected \"";
            cout << output;
            cout << expected[i];
            output = fr ? "\"\nSortie obtenue: \"" : "\"\nGot \"";
            cout << output;
            cout << result[i];
            cout << "\"";
        }
    }

    // Tracking of total number of successful tests / Suivi du nombre total de tests réussis
    cout << "\n\n"
         << nbr_tests_passed << "/" << nbr_cases;
    output = fr ? " tests complétés.\n" : " successful test cases.\n";
    cout << output;
    if (nbr_tests_passed == nbr_cases) {
        output = fr ? "Tout a l'air bon!\n" : "Everything seems good!\n";
    } else {
        output = fr ? "Il semble y avoir un problème avec certaines sorties." : "There seems to be a problem in the outputs somewhere.";
    }
    cout << output;
}

// DO NOT TOUCH! / NE PAS TOUCHER!
int main(int argc, char* argv[])
{
    const int nbr_cases = 4;

    // inputs / entrées
    string TEST_CASES[nbr_cases] = {
        "dans l'ordre naturel des choses, 22+20 a toujours ete un nombre "
        "important. peu importe le contexte, 42 represente la verite vraie.",
        "etant riche de 3*5 dollars, julie peut s'acheter les bonbons de son "
        "choix! son ami jimmy, lui, possede 14-12 dollars et n'aura donc pas ses "
        "bonbons desires. il devra se contenter d'acheter de la gomme.",
        "charles goes to his school every morning in montreal, quebec, knowing "
        "that his dreams are about to be fulfilled.",
        "what a beautiful day outside! it's almost like louis could touch some "
        "grass! 20/5 kilos on his lawn, to be precise."
    };
    /* Looks like this in problem's description: / A l'air de ça dans la
    description du problème: dans l'ordre naturel des choses, 22+20 a toujours ete
    un nombre important. peu importe le contexte, 42 represente la verite vraie.
    etant riche de 3*5 dollars, julie peut s'acheter les bonbons de son choix! son
    ami jimmy, lui, possede 14-12 dollars et n'aura donc pas ses bonbons desires.
    il devra se contenter d'acheter de la gomme. charles goes to his school every
    morning in montreal, quebec, knowing that his dreams are about to be
    fulfilled. what a beautiful day outside! it's almost like louis could touch
    some grass! 20/5 kilos on his lawn, to be precise.
    */

    // expected outputs / sorties attendue
    string EXPECTED_ANSWERS[nbr_cases] = {
        "Dans l'or_dre naturel des cho_ses, 42 a touj_ours ete\tu_n\tnom_bre important. Peu importe l_e cont_exte, 42 repre_sente\tl_a ver_ite vraie.\t",
        "Etant riche d_e 15 dollars, Julie pe_ut s'ach_eter les bonbons d_e son choix! Son ami Jimmy, lui, possede 2 dollars e_t\tn'aura\tdo_nc pas ses bonbons desires. Il devra\ts_e contenter d'ach_eter d_e l_a gomme.",
        "Charles go_es t_o\this sch_ool every\tmorning\ti_n Montreal, Quebec, knowing th_at his dre_ams\tare about t_o\tb_e fulfilled.",
        "What a beautiful day\toutside! It's alm_ost\tli_ke Louis could\ttouch so_me grass! 4 kilos o_n his la_wn,\tt_o	b_e precise.\t"
    };
    /* Should look like this after the checkAnswer() method: / Devrait ressembler
    à ça après la fonction checkAnswer(): Dans l'or_dre naturel des cho_ses, 42 a
    touj_ours ete	u_n	nom_bre important. Peu importe l_e cont_exte, 42
    repre_sente	l_a ver_ite vraie.
    Etant	riche d_e 15 dollars, Julie pe_ut s'ach_eter les bonbons d_e son choix!
    Son ami Jimmy, lui, possede 2 dollars e_t	n'aura	do_nc pas ses bonbons
    desires. Il devra	s_e contenter d'ach_eter d_e l_a gomme. Charles go_es
    t_o	his sch_ool every	morning	i_n Montreal, Quebec, knowing th_at his
    dre_ams	are about t_o	b_e fulfilled. Wh_at a beautiful day	outside!
    It's alm_ost	li_ke Louis could	touch so_me grass! 4 kilos o_n his
    la_wn,	t_o	b_e precise.
    */

    using Clock = std::chrono::high_resolution_clock;
    using Ms = std::chrono::duration<float, std::chrono::milliseconds::period>;

    auto start = Clock::now();

    string result[nbr_cases] = {};
    for (int i = 0; i < nbr_cases; i++) {
        result[i] = solve(TEST_CASES[i]);
    }

    auto duration = Clock::now() - start;

    checkAnswers(result, EXPECTED_ANSWERS, nbr_cases);

    std::cout << "\nSolve Time: "
              << std::chrono::duration_cast<Ms>(duration).count()
              << " ms"
              << std::endl;
}
