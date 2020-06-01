#include "AST.h"
#include "checker.h"
#include "history.h"
#include "parser.h"
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  char *line = NULL;

  while (line = readline("sosh> ")) {
    // Si no es una linea vacia
    history_add(line);
    read_from(line);
    ASTNode *root = parse();
    if (root) {
      root = remove_G(root);
      print_node(root, 0);
      Error err;
      if (check_redirections(root, &err) < 0) {
        printf("Error while checking redirections\n");
      }
      print_node(root, 0);
      free_node(root);
    }
  }

  history_delete();
  printf("\n");
  return 0;
}