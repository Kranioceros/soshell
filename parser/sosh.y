%{
    #include <assert.h>
    #include <stdio.h>
    #include <string.h>

%}

%code requires {
    #include "../src/AST.h"
    #include "../src/parser.h"

    static ASTNode* root = NULL;
}

%union {
    char* s;
    Params* p;
    Commands* c;
    ASTNode *n;
}

%token <s> SIMPLE_STRING
%token <s> BEGIN_DELIM_STRING
%token EOL

%type <s> string
%type <p> params
%type <c> pipe_list
%type <n> single_call call_from_file call_to_file call_from_to_file
%type <n> call compound_call simple_call ungrouped_simple_call

%%

test:
    call EOL { 
                root = $1;
             }
    ;

call:
    simple_call
    | compound_call

compound_call:
    pipe_list simple_call { if ($2->value.scall.stdin_to != NULL)  {
                                yyerror("last call inside pipeline contains stdin redirection");
                                YYERROR;
                            }
                            add_comm($1, $2);
                            $$ = new_ccall('|', $1); }
    ;

pipe_list:
    simple_call '|'                   { if ($1->value.scall.stdout_to != NULL) {
                                            yyerror("first call inside pipeline contains stdout redirection");
                                            YYERROR;
                                        }
                                        $$ = new_comm($1); }
    | pipe_list simple_call '|'       { if ($2->value.scall.stdout_to != NULL || $2->value.scall.stdin_to != NULL) {
                                            yyerror("call inside pipeline contains redirection of stdin/stdout");
                                            YYERROR;
                                        }
                                        add_comm($1, $2);
                                        $$ = $1; }
    ;

simple_call:
    '[' simple_call ']'  { $$ = $2; }
    | ungrouped_simple_call
    ;

ungrouped_simple_call:
    call_from_to_file
    | call_from_file
    | call_to_file
    | single_call
    ;

call_from_to_file:
    '<' string single_call '>' string { printf("call_from_to_file\n");
                                        stdout_to_file($3, $5);
                                        stdin_to_file($3, $2);
                                        $$ = $3; }
    ;

call_to_file:
    single_call '>' string { printf("call_to_file\n");
                             stdout_to_file($1, $3);
                             $$ = $1; }
    ;

call_from_file:
    '<' string single_call { printf("call_from_file\n");
                             stdin_to_file($3, $2);
                             $$ = $3; }
    ;

single_call:
    string          { printf("single_call\n"); $$ = new_scall($1, NULL); }
    | string params { printf("single_call\n"); $$ = new_scall($1, $2); }
    ;

params:
    string          { $$ = new_param($1); }
    | params string { add_param($1, $2);
                      $$ = $1; }
    ;

string: SIMPLE_STRING           { $$ = strdup($1); }
    | BEGIN_DELIM_STRING '}'    { const size_t len = strlen($1) - 2; // Restamos las dos llaves
                                  $$ = malloc(len + 1);              // Sumamos 1 para el \0
                                  strncpy($$, $1 + 1, len);          // Copiamos desde despues desde {,
                                                                     // hasta antes de }
                                  $$[len] = '\0';                    // Ponemos el \0 no copiado
                                }
    ;

%%
ASTNode* parse() {
    yyparse();
    return root;
}

int yyerror(const char *s) {
    fprintf(stderr, "error: %s\n", s);
}