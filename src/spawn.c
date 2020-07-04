#include "spawn.h"
#include "AST.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

TySpawnError run_scall(SimpleCall *call, int *status_code) {
  assert(call);
  assert(call->program_name);

  const char **args;

  // Creamos el vector de argumentos
  if (call->params) {
    char argc = call->params->count + 1;
    args = malloc(sizeof(char *) * (argc + 1));
    args[0] = call->program_name;
    for (int i = 1; i < argc + 1; ++i) {
      args[i] = call->params->s[i - 1];
    }
    args[argc] = (char *)NULL;
  } else {
    (char *)args[2];
    args[0] = call->program_name;
    args[1] = (char *)NULL;
  }
  // Copiamos stdin y stdout para restaurarlos luego
  int old_stdin = dup(STDIN_FILENO);
  int old_stdout = dup(STDOUT_FILENO);

  // Redireccionamos
  int new_stdin = -1;
  int new_stdout = -1;

  if (call->stdin_to) {
    new_stdin = open(call->stdin_to, O_RDONLY);
    if (new_stdin < 0) {
      switch (errno) {
      case EACCES:
        return (ERR_ACCESO_IN);
      case ENOENT:
        return (ERR_NOEXISTE_IN);
      default:
        return (ERR_DESCONOCIDO);
      }
    } else {
      dup2(new_stdin, STDIN_FILENO);
      close(new_stdin);
    }
  }

  if (call->stdout_to) {
    new_stdout = open(call->stdout_to, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (new_stdout < 0) {
      switch (errno) {
      case EACCES:
        return ERR_ACCESO_OUT;
      case ENOENT:
        return ERR_NOEXISTE_OUT;
      default:
        return ERR_DESCONOCIDO;
      }
    } else {
      dup2(new_stdout, STDOUT_FILENO);
      close(new_stdout);
    }
  }

  // Hacemos el fork
  pid_t p = fork();

  switch (p) {
  case 0:
    // Ejecutamos el programa
    execvp(call->program_name, (char *const *)args);

    // Si retorno es porque hubo un error
    switch (errno) {
    case EACCES:
      return ERR_ACCESO_EXEC;
    case ENOENT:
      return ERR_NOEXISTE_EXEC;
    default:
      return ERR_DESCONOCIDO;
    }

  case -1:
    // Restauramos descriptores
    dup2(old_stdin, STDIN_FILENO);
    close(old_stdin);
    dup2(old_stdout, STDOUT_FILENO);
    close(old_stdout);

    printf("Error al crear proceso hijo\n");
    return ERR_DESCONOCIDO;

  default:
    wait(status_code);

    // Restauramos descriptores
    dup2(old_stdin, STDIN_FILENO);
    close(old_stdin);
    dup2(old_stdout, STDOUT_FILENO);
    close(old_stdout);

    if (WIFEXITED(*status_code)) {
      printf("Todo en orden\n");
      return 0;
    }
  }

  return 0;
}

void print_spawn_error(TySpawnError type, ASTNode *node) {
  switch (type) {
  case ERR_ACCESO_EXEC:
    printf("El acceso al programa '%s' o alguno de los directorios que lo "
           "contiene no es posible\n",
           node->value.scall.program_name);
    break;
  case ERR_NOEXISTE_EXEC:
    printf("El programa '%s' no existe\n", node->value.scall.program_name);
    break;
  case ERR_ACCESO_IN:
    printf("El acceso al archivo de entrada '%s' o alguno de los "
           "directorios que lo "
           "contiene no es posible\n",
           node->value.scall.stdin_to);
    break;
  case ERR_NOEXISTE_IN:
    printf("El archivo de entrada '%s' no existe\n",
           node->value.scall.stdin_to);
    break;
  case ERR_ACCESO_OUT:
    printf("El acceso al archivo de salida '%s' o alguno de los "
           "directorios que lo "
           "contiene no es posible\n",
           node->value.scall.stdout_to);
    break;
  case ERR_NOEXISTE_OUT:
    printf("El archivo de salida '%s' no existe\n",
           node->value.scall.stdout_to);
    break;
  case ERR_DESCONOCIDO:
    printf("Hubo un error desconocido\n");
    break;
  }
}