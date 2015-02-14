%option noyywrap
%{
/* * * * * * * * * * * *
 * * * DEFINITIONS * * *
 * * * * * * * * * * * */
%}

%{
// recognize whether or not a credit card number is valid
int line_num = 1;
%}

digit [0-9]
group {digit}{4}
alpha [a-zA-Z0-9_]
Beta ([s|S][e|E][l|L][e|E][c|C][t|T]" ""*"" "[f|F][r|R][o|O][m|M]" "{alpha}*)
Gamma {alter" "{alpha}*}
%%

%{
/* * * * * * * * * 
 * * * RULES * * *
 * * * * * * * * */
%}
   /* The carat (^) says that a credit card number must start at the
      beginning of a line and the $ says that the credit card number
      must end the line. */
[s|S][e|E][l|L][e|E][c|C][t|T]" ""*"" "[f|F][r|R][o|O][m|M]" "{alpha}*|alter" "{alpha}*
   /* The .* accumulates all the characters on any line that does not
      match a valid credit card number */
.* { printf("%d: error: %s \n", line_num, yytext); }
\n { line_num++; }
%%

/* * * * * * * * * * * 
 * * * USER CODE * * *
 * * * * * * * * * * *
 */
int main(int argc, char *argv[]) {
  yylex();
}
