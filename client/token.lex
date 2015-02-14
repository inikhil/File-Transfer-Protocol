%option noyywrap
%{
#include "ex1.tab.hpp"
#include <iostream>
using namespace std;
int line_num = 1;
%}

digit [0-9]
alpha [a-zA-Z0-9_]
alp [a-zA-Z]
space [" "]
comp "<="|">="|"<"|">"|"="|"=="|"<>"|"!="
oper "+"|"-"|"/"|"*"
sel [s|S][e|E][l|L][e|E][c|C][t|T]
cre [c|C][r|R][e|E][a|A][t|T][e|E]
tab [t|T][a|A][b|B][l|L][e|E]
dat [d|D][a|A][t|T][a|A][b|B][a|A][s|S][e|E]
cre_db {cre}{space}+{dat}{space}+{alpha}+";"
var_ty [i|I][n|N][t|T]|[c|C][h|H][a|A][r|R]|[c|C][h|H][a|A][r|R]"("{digit}+")"|[v|V][a|A][R|r][c|C][h|H][a|A][r|R]"("{digit}+")"|[v|V][a|A][R|r][c|C][h|H][a|A][r|R]|[f|F][l|L][o|O][a|A][t|T]|[D|d][o|O][u|U][b|B][l|L][e|E]|[d|D][a|A][t|T][e|E]
var_col {alpha}+{space}+{var_ty}
cre_tb {cre}{space}+{tab}{space}+{alpha}+{space}*"("{space}*({var_col}{space}*","{space}*)*{var_col}{space}*")"";"
use_db [u|U][s|S][e|E]{space}+{alpha}+";"
show_tb [s|S][h|H][o|O][w|W]{space}+{tab}[s|S]";"
show_db [s|S][h|H][o|O][w|W]{space}+{dat}[s|S]";"
val {digit}*"."{digit}+|{digit}+|{alp}+|{digit}{2}"/"{digit}{2}"/"{digit}{4}
ins [i|I][n|N][s|S][e|E][r|R][t|T]{space}+[i|I][n|N][t|T][o|O]{space}+{alpha}*{space}+[v|V][a|A][l|L][u|U][e|E][s|S]{space}*"("({space}*{val}{space}*",")*{space}*{val}")"";"
drop_tb [d|D][r|R][o|O][p|P]{space}+{tab}{space}+{alpha}*";"
drop_db [d|D][r|R][o|O][p|P]{space}+{dat}{space}+{alpha}*";"
desc_tb ([d|D][e|E][s|S][c|C]|[d|D][e|E][s|S][c|C][r|R][i|I][b|B][e|E]){space}+{alpha}+;
del_wh {alpha}+{space}*{comp}{space}*{val}
del [d|D][e|E][l|L][e|E][t|T][e|E]{space}+[f|F][r|R][o|O][m|M]{space}+{alpha}*{space}+[w|W][h|H][e|E][r|R][e|E]{space}+({del_wh}*{space}*","{space}*)*{del_wh}{space}*";"
sel_val {val}{space}*{comp}{space}*{alpha}+|{alpha}+{space}*{comp}{space}*{val}
sim_sel {sel}{space}+("*"|{alpha}+){space}+[f|F][r|R][o|O][m|M]{space}+{alpha}+
whe_sel {sim_sel}{space}+[w|W][h|H][e|E][r|R][e|E]{space}+{sel_val};
sim_upd [u|U][p|P][d|D][a|A][t|T][e|E]{space}+{alpha}+{space}+[s|S][e|E][t|T]{space}+{alpha}+{space}*"="{space}*{val}
whe_upd {sim_upd}{space}+[w|W][h|H][e|E][r|R][e|E]{space}+{sel_val};

%%

[0-9]+   { yylval.val = atoi(yytext); return NUM; }
[\+|\-]  { yylval.sym = yytext[0]; return OPA; }
[\*|/]   { yylval.sym = yytext[0]; return OPM; }
"("      { return LP; }
")"      { return RP; }
";"      { return STOP; }
<<EOF>>  { return 0; }
[ \t\n]+ { }
.        { cerr << "Unrecognized token!" << endl; exit(1); }

%%

/* * * * * * * * * * * 
 * * * USER CODE * * *
 * * * * * * * * * * *
 */

