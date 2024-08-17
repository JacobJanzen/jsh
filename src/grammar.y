%{
#include <stdio.h>
#include "../src/lexer.h"
%}

%define api.value.type {char *}
%token WORD
%token ASSIGNMENT_WORD
%token NAME
%token NEWLINE
%token IO_NUMBER

/* operators */
%token AND_IF
%token OR_IF
%token DSEMI
%token DLESS
%token DGREAT
%token LESSAND
%token GREATAND
%token LESSGREAT
%token DLESSDASH
%token CLOBBER

/* reserved words */
%token If
%token Then
%token Else
%token Elif
%token Fi
%token Do
%token Done
%token Case
%token Esac
%token While
%token Until
%token For
%token Lbrace
%token Rbrace
%token Bang
%token In

%%
default : If
        ;

%%
