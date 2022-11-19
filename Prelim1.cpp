#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
// L'anglais est mis par defaut. Si vous voulez un affichage en francais,
// changez le string de la ligne suivante pour "FR"
string CONSOLE_LANGUAGE = "ENG";
// DO NOT DELETE STATEMENT / NE PAS EFFACER CETTE DÉCLARATION

string solve(string testCase) {
  string answer; // placeholder
  // Code goes here / Votre code commence ici

  return answer; // Return test case output here / Retourner la sortie de votre
                 // code sur cette ligne
}

// Do not modify anything beyond this point. / Ne rien modifier après ce point.
// The rest of this code is to facilitate the validation and testing of your
// code. Le reste de ce code sert à faciliter la validation et le test de votre
// code.

// Function to show if your code works / Fonction pour vous montrer si votre
// code fonctionne
void checkAnswers(string *result, string *expected, int nbr_cases) {
  int nbr_tests_passed = 0;
  bool fr = CONSOLE_LANGUAGE.compare("FR") == 0;
  string output = fr ? "Votre Sortie:\n" : "Your Output:\n";
  cout << output;

  // #Quick print to show all your code's outputs on different lines / Affichage
  // rapide pour vous montrer vos sorties
  for (int i = 0; i < nbr_cases; i++) {
    cout << result[i] << "\n";
  }

  // To tell you if a testcase is successful / Pour vous dire si un test est
  // réussi
  for (int i = 0; i < nbr_cases; i++) {
    output = fr ? "\n\nTest " : "\n\nCase ";
    cout << output;
    cout << i + 1;
    if (result[i].compare(expected[i]) == 0) {
      output = fr ? " réussi!" : " successful!";
      cout << output;
      nbr_tests_passed++;
    } else {
      output = fr ? " non réussi.\nOn s'attendait à: \""
                  : " unsuccessful.\nExpected \"";
      cout << output;
      cout << expected[i];
      output = fr ? "\"\nSortie obtenue: \"" : "\"\nGot \"";
      cout << output;
      cout << result[i];
      cout << "\"";
    }
  }

  // Tracking of total number of successful tests / Suivi du nombre total de
  // tests réussis
  cout << "\n\n" << nbr_tests_passed << "/" << nbr_cases;
  output = fr ? " tests complétés.\n" : " successful test cases.\n";
  cout << output;
  if (nbr_tests_passed == nbr_cases) {
    output = fr ? "Tout a l'air bon!\n" : "Everything seems good!\n";
  } else {
    output = fr ? "Il semble y avoir un problème avec certaines sorties."
                : "There seems to be a problem in the outputs somewhere.";
  }
  cout << output;
}

// DO NOT TOUCH! / NE PAS TOUCHER!
int main(int argc, char *argv[]) {
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
      "grass! 20/5 kilos on his lawn, to be precise."};
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
      "Dans l'or_dre naturel des cho_ses, 42 a touj_ours ete\tu_n\tnom_bre "
      "important. Peu importe l_e cont_exte, 42 repre_sente\tl_a ver_ite "
      "vraie.\t",
      "Etant\triche d_e 15 dollars, Julie pe_ut s'ach_eter les bonbons d_e son "
      "choix! Son ami Jimmy, lui, possede 2 dollars e_t\tn'aura\tdo_nc pas ses "
      "bonbons desires. Il devra\ts_e contenter d'ach_eter d_e l_a gomme.",
      "Charles go_es t_o\this sch_ool every\tmorning\ti_n Montreal, Quebec, "
      "knowing th_at his dre_ams\tare about t_o\tb_e fulfilled.",
      "Wh_at a beautiful day\toutside! It's alm_ost\tli_ke Louis could\ttouch "
      "so_me grass! 4 kilos o_n his la_wn,\tt_o	b_e precise.\t"};
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

  string result[nbr_cases] = {};
  for (int i = 0; i < nbr_cases; i++) {
    result[i] = solve(TEST_CASES[i]);
  }
  checkAnswers(result, EXPECTED_ANSWERS, nbr_cases);
}
