#include "AST.h"
#include "checker.h"
#include "history.h"
#include "parser.h"
#include "spawn.h"
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>

// Contiene booleano seteado por yyerror; establece
// si hubo un error en el parsing y si es seguro
// liberar la memoria del arbol o no
int parse_error = 0;
// Contiene puntero al nodo raiz del arbol parseado
ASTNode *root = NULL;

int main(int argc, char *argv[]) {
  char *line = NULL;

  while (line = readline("sosh> ")) {
    // Si no es una linea vacia
    if (strlen(line) == 0) {
      continue;
    }

    history_add(line);
    read_from(line);
    root = NULL;
    parse_error = 0;
    root = parse();
    if (!parse_error) {
      root = remove_G(root);
      // Verificamos que no haya errores de redireccion de la salida/entrada
      // estandar
      RedirError err_re;
      err_re.type = NO_REDIRERR;
      if (check_redirections(root, &err_re) != NO_ERR) {
        print_redirerror(&err_re);
        free_node(root);
        continue;
      }

      print_node(root, 0);

      int status_code;
      SpawnError err_sp;
      err_sp.type = NO_ERR;
      int spawn_error;

      if (root->type == TyCompoundCall) {
        spawn_error = run_ccall(&root->value.ccall, &status_code, &err_sp);
      } else {
        spawn_error = run_scall(&root->value.scall, &status_code, &err_sp);
      }

      // Mostramos error de spawn si lo hubo
      if (spawn_error != 0) {
        print_spawn_error(&err_sp);
      }

      free_node(root);
    }
  }

  history_delete();
  printf("\n");
  return 0;
}