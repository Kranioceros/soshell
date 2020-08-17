%{
    #include <assert.h>
    #include <stdio.h>
    #include <string.h>
%}

%code requires {
    #include "../src/AST.h"
    #include "../src/parser.h"

    extern ASTNode* root;
    extern int parse_error;
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
%type <n> single_call call_from_file call_to_file call_from_to_file
%type <n> simple_call ungrouped_simple_call
%type <n> call compound_call pipe

%left '|'

%%

sosh:
    call EOL { 
                root = $1;
             }
    ;

call:
    simple_call
    | compound_call
    ;

compound_call:
    pipe                            { $$ = $1; }
    | '[' compound_call ']'         { $$ = $2; }
    ;

pipe:
    simple_call '|' simple_call {
                                    debugf("scall | scall\n");
                                    Commands *c = new_comm($1);
                                    add_comm(c, $3);
                                    $$ = new_ccall('|', c);
                                }
    | simple_call '|' compound_call {
                                    debugf("scall | ccall\n");
                                    CompoundCall *ccall = &$3->value.ccall;
                                    switch(ccall->op) {
                                        case 'G':
                                        ccall->op = '|';
                                        add_comm_begin(ccall->comms, $1);
                                        break;

                                        case '|':
                                        add_comm_begin(ccall->comms, $1);
                                        break;

                                        default:
                                        yyerror("WTF\n");
                                    }
                                    $$ = $3;
                                    }
    | compound_call '|' compound_call {
                                    debugf("ccall | ccall\n");
                                    CompoundCall *ccall = &$1->value.ccall;
                                    switch(ccall->op) {
                                        case 'G':
                                        ccall->op = '|';
                                        add_comm(ccall->comms, $3);
                                        break;

                                        case '|':
                                        add_comm(ccall->comms, $3);
                                        break;

                                        default:
                                        yyerror("WTF\n");
                                    }
                                    }
    | compound_call '|' simple_call {
                                    debugf("ccall | scall\n");
                                    CompoundCall *ccall = &$1->value.ccall;
                                    switch(ccall->op) {
                                        case 'G':
                                        ccall->op = '|';
                                        add_comm(ccall->comms, $3);
                                        break;

                                        case '|':
                                        add_comm(ccall->comms, $3);
                                        break;

                                        default:
                                        yyerror("WTF\n");
                                    }
                                    }
    ;

simple_call:
    '[' simple_call ']'             { $$ = $2; }
    | ungrouped_simple_call
    ;

ungrouped_simple_call:
    call_from_to_file
    | call_from_file
    | call_to_file
    | single_call
    ;

call_from_to_file:
    '<' string single_call '>' string { stdout_to_file($3, $5);
                                        stdin_to_file($3, $2);
                                        $$ = $3; }
    ;

call_to_file:
    single_call '>' string { stdout_to_file($1, $3);
                             $$ = $1; }
    ;

call_from_file:
    '<' string single_call { stdin_to_file($3, $2);
                             $$ = $3; }
    ;

single_call:
    string          { $$ = new_scall($1, NULL); }
    | string params { $$ = new_scall($1, $2); }
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
    fprintf(stderr, "Error de sintaxis\n", s);
    parse_error = 1;
}