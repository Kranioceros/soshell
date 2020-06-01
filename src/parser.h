#ifndef PARSER_H
#define PARSER_H
#include "../src/AST.h"
#include <stdio.h>
// Este archivo contiene variables y funciones globales
// del parser y el lexer.
#define YY_DECL int yylex(void)

// Resetea el buffer de lectura del lexer y lo hace leer de `line`
extern void read_from(const char *line);
// Instruccion principal para parsear un nodo
extern ASTNode *parse();
// Imprime mensaje de error en el parser
extern int yyerror(const char *s);
// Para suprimir advertencia en el compilador
extern YY_DECL;

#endif