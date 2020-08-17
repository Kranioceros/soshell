#include "AST.h"
#include "checker.h"
#include "history.h"
#include "parser.h"
#include "spawn.h"
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  char *line = NULL;

  while (line = readline("sosh> ")) {
    // Si no es una linea vacia
    if (strlen(line) == 0) {
      continue;
    }

    history_add(line);
    read_from(line);
    ASTNode *root = parse();
    if (root) {
      root = remove_G(root);
      // print_node(root, 0);
      Error err;
      if (check_redirections(root, &err) < 0) {
        printf("Error while checking redirections\n");
      }
      // print_node(root, 0);

      int status_code;
      SpawnError err_sp;
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