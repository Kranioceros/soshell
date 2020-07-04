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
      print_node(root, 0);

      // Exceptuamos comandos compuestos por ahora
      if (root->type == TyCompoundCall) {
        printf("Aun no soportamos comandos compuestos :)\n");
        free_node(root);
        continue;
      }

      // Ejecutamos comando (bloquea el proceso)
      printf("=======================\n");
      int status_code;
      TySpawnError res = run_scall(&root->value.scall, &status_code);

      // Mostramos error si lo hubo
      if (res != 0) {
        print_spawn_error(res, root);
      }

      free_node(root);
    }
  }

  history_delete();
  printf("\n");
  return 0;
}