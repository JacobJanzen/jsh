#ifndef LEXER_H_
#define LEXER_H_

#include <stdio.h>

int yylex(void);
void yyerror(char const *);
int yyparse_wrapper(FILE *);

#endif /* LEXER_H_ */
