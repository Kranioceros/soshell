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

// Funciones auxiliares

// Cambia stdin y stdout -> hace fork -> hace exec -> restaura stdin y stdout
// Los descripitores in_fd y out_fd son cerrados por esta funcion
// Tras finalizar la funcion, stdin y stdout se encuentran en su estado original
// Devuelve pid_t del proceso creado. Si hay error, pid = 0 y se guarda error en
// err.
pid_t exec_scall(const char *program_name, Params *params, int in_fd,
                 int out_fd, SpawnError *err);
// Funciona de modo analogo a exec_scall, pero ahora el proceso hijo es un
// proceso que espera la ejecucion de todos los subcomandos
pid_t exec_ccall(Commands *comms, int in_fd, int out_fd, SpawnError *err);

pid_t run_pipe(Commands *comms, SpawnError *err);

// FUNCIONES PRINCIPALES
// Ejecuta la llamada call. Cualquier error relacionado a permisos de los
// archivos o existencia de los mismos se devuelve en err. Si no hay errores
// asi, devuelve 0.
// Cualquier error del programa en si va en status_code. Esto es mucho mas
// facil decirlo que hacerlo
int run_scall(SimpleCall *call, int *status_code, SpawnError *err) {
  assert(call);
  assert(call->program_name);
  assert(err);

  err->type = NO_ERR;

  // Abrimos archivos para redireccionar stdin y stdout
  int new_stdin = -1;
  int new_stdout = -1;

  if (call->stdin_to) {
    new_stdin = open(call->stdin_to, O_RDONLY);
    if (new_stdin < 0) {
      switch (errno) {
      case EACCES:
        err->str = call->stdin_to;
        err->type = ERR_ACCESO_IN;
        return -1;
      case ENOENT:
        err->str = call->stdin_to;
        err->type = ERR_NOEXISTE_IN;
        return -1;
      default:
        err->type = ERR_DESCONOCIDO;
        return -1;
      }
    }
  }

  if (call->stdout_to) {
    new_stdout = open(call->stdout_to, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (new_stdout < 0) {
      switch (errno) {
      case EACCES:
        err->str = call->stdout_to;
        err->type = ERR_ACCESO_OUT;
        return -1;
      case ENOENT:
        err->str = call->stdout_to;
        err->type = ERR_NOEXISTE_OUT;
        return -1;
      default:
        err->type = ERR_DESCONOCIDO;
        return -1;
      }
    }
  }

  pid_t hijo =
      exec_scall(call->program_name, call->params, new_stdin, new_stdout, err);

  // Si hubo un error al crear proceso o algunos de los archivos de stdin o
  // stdout no existe, se retorna sin esperar al hijo
  if (hijo == 0) {
    debugf("hijo == 0\n");
    return -1;
  }

  wait(status_code);

  // Procesamos status_code
  int decoded = WEXITSTATUS(*status_code);
  int error_exec =
      (decoded == ERR_NOEXISTE_EXEC) || (decoded == ERR_ACCESO_EXEC);

  if (WIFEXITED(*status_code) && !error_exec) {
    // Si el hijo termino normalmente y el exec se ejecuto sin errores
    debugf("hijo se ejecuto y retorno exitosamente\n");
    return 0;
  } else if (WIFEXITED(*status_code) && error_exec) {
    // Si el hijo termino normalmente pero el exec fallo
    // (Esto asume que el programa a ejecutar no usa los codigos de errores 126
    // y 127. No deberian!)
    debugf("hijo no se ejecutó exitosamente\n");
    err->type = decoded;
    err->str = call->program_name;
    return -1;
  } else {
    // Si el hijo no termino normalmente, pero no tiene que ver con el
    // ejecutable (?)
    debugf("hijo termino abruptamente (?)\n");
    err->type = ERR_DESCONOCIDO;
    return -1;
  }
}

int run_ccall(CompoundCall *call, int *status_code, SpawnError *err) {
  assert(call);
  assert(call->comms);

  // Nos aseguramos de que no se usen llamadas compuestas anidadas
  // (sin implementar aun)
  for (int i = 0; i < call->comms->count; ++i) {
    if (call->comms->c[i]->type == TyCompoundCall) {
      err->type = ERR_ANIDADAS;
      err->str = NULL;
      return -1;
    }
  }

  // Abrimos archivos para redireccionar stdin y stdout
  int new_stdin = -1;
  int new_stdout = -1;

  if (call->stdin_to) {
    new_stdin = open(call->stdin_to, O_RDONLY);
    if (new_stdin < 0) {
      switch (errno) {
      case EACCES:
        err->str = call->stdin_to;
        err->type = ERR_ACCESO_IN;
        return -1;
      case ENOENT:
        err->str = call->stdin_to;
        err->type = ERR_NOEXISTE_IN;
        return -1;
      default:
        err->type = ERR_DESCONOCIDO;
        return -1;
      }
    }
  }

  if (call->stdout_to) {
    new_stdout = open(call->stdout_to, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (new_stdout < 0) {
      switch (errno) {
      case EACCES:
        err->str = call->stdout_to;
        err->type = ERR_ACCESO_OUT;
        return -1;
      case ENOENT:
        err->str = call->stdout_to;
        err->type = ERR_NOEXISTE_OUT;
        return -1;
      default:
        err->type = ERR_DESCONOCIDO;
        return -1;
      }
    }
  }

  pid_t hijo = exec_ccall(call->comms, new_stdin, new_stdout, err);

  // Si hubo un error al crear proceso o algunos de los archivos de stdin o
  // stdout no existe, se retorna sin esperar al hijo
  if (hijo == 0) {
    debugf("hijo == 0\n");
    return -1;
  }

  wait(status_code);

  // Procesamos status_code
  int decoded = WEXITSTATUS(*status_code);
  int error_exec =
      (decoded == ERR_NOEXISTE_EXEC) || (decoded == ERR_ACCESO_EXEC);

  if (WIFEXITED(*status_code) && !error_exec) {
    // Si el hijo termino normalmente y el exec se ejecuto sin errores
    debugf("hijo se ejecuto y retorno exitosamente\n");
    return 0;
  } else if (WIFEXITED(*status_code) && error_exec) {
    // Si el hijo termino normalmente pero el exec fallo
    // (Esto asume que el programa a ejecutar no usa los codigos de errores 126
    // y 127. No deberian!)
    debugf("hijo no se ejecutó exitosamente\n");
    err->type = decoded;
    err->str = "PIPELINE";
    return -1;
  } else {
    // Si el hijo no termino normalmente, pero no tiene que ver con el
    // ejecutable (?)
    debugf("hijo termino abruptamente (?)\n");
    err->type = ERR_DESCONOCIDO;
    return -1;
  }

  return 0;
}

void print_spawn_error(SpawnError *err) {
  assert(err);
  switch (err->type) {
  case ERR_ACCESO_EXEC:
    printf("El acceso al programa '%s' o alguno de los directorios que lo "
           "contiene no es posible\n",
           err->str);
    break;
  case ERR_NOEXISTE_EXEC:
    printf("El programa '%s' no existe\n", err->str);
    break;
  case ERR_ACCESO_IN:
    printf("El acceso al archivo de entrada '%s' o alguno de los "
           "directorios que lo "
           "contiene no es posible\n",
           err->str);
    break;
  case ERR_NOEXISTE_IN:
    printf("El archivo de entrada '%s' no existe\n", err->str);
    break;
  case ERR_ACCESO_OUT:
    printf("El acceso al archivo de salida '%s' o alguno de los "
           "directorios que lo "
           "contiene no es posible\n",
           err->str);
    break;
  case ERR_NOEXISTE_OUT:
    printf("El archivo de salida '%s' no existe\n", err->str);
    break;
  case ERR_PIPE:
    printf("Hubo un error creando pipes (probablemente se excedio el limite "
           "maximo de descriptores de archivo por proceso)\n");
    break;
  case ERR_ANIDADAS:
    printf("El comando ingresado contiene llamadas compuestas anidadas; sosh "
           "aun no"
           " soporta esta caracteristica\n");
    break;
  case ERR_DESCONOCIDO:
    printf("Hubo un error desconocido\n");
    break;
  }
}

// FUNCIONES AUXILIARES

pid_t exec_scall(const char *program_name, Params *params, int in_fd,
                 int out_fd, SpawnError *err) {
  assert(program_name);
  assert(err);

  const char **args;

  // Creamos el vector de argumentos
  if (params) {
    char argc = params->count + 1;
    args = malloc(sizeof(char *) * (argc + 1));
    args[0] = program_name;
    for (int i = 1; i < argc + 1; ++i) {
      args[i] = params->s[i - 1];
    }
    args[argc] = (char *)NULL;
  } else {
    args = malloc(sizeof(char *) * 2);
    args[0] = program_name;
    args[1] = (char *)NULL;
  }

  // Copiamos stdin y stdout para restaurarlos luego
  int old_stdin = dup(STDIN_FILENO);
  int old_stdout = dup(STDOUT_FILENO);

  // Redireccionamos entrada y salida estandar. Cerramos in_fd y out_fd
  if (in_fd > -1) {
    dup2(in_fd, STDIN_FILENO);
    close(in_fd);
  }

  if (out_fd > -1) {
    dup2(out_fd, STDOUT_FILENO);
    close(out_fd);
  }

  // Hacemos el fork
  pid_t p = fork();

  switch (p) {
  case 0:
    // Ejecutamos el programa
    execvp(program_name, (char *const *)args);

    // Si retorno es porque hubo un error, lo comunicamos al padre
    switch (errno) {
    case EACCES:
      _exit(ERR_ACCESO_EXEC);
    case ENOENT:
      _exit(ERR_NOEXISTE_EXEC);
    default:
      _exit(ERR_DESCONOCIDO);
    }

  case -1:
    // Restauramos descriptores
    dup2(old_stdin, STDIN_FILENO);
    close(old_stdin);
    dup2(old_stdout, STDOUT_FILENO);
    close(old_stdout);

    debugf("Error al crear proceso hijo\n");
    err->type = ERR_DESCONOCIDO;

  default:
    // Restauramos descriptores
    dup2(old_stdin, STDIN_FILENO);
    close(old_stdin);
    dup2(old_stdout, STDOUT_FILENO);
    close(old_stdout);

    return p;
  }
}

pid_t exec_ccall(Commands *comms, int in_fd, int out_fd, SpawnError *err) {
  assert(comms);
  assert(err);

  // Copiamos stdin y stdout para restaurarlos luego
  int old_stdin = dup(STDIN_FILENO);
  int old_stdout = dup(STDOUT_FILENO);

  // Redireccionamos entrada y salida estandar. Cerramos in_fd y out_fd
  if (in_fd > -1) {
    dup2(in_fd, STDIN_FILENO);
    close(in_fd);
  }

  if (out_fd > -1) {
    dup2(out_fd, STDOUT_FILENO);
    close(out_fd);
  }

  // Hacemos el fork
  pid_t p = fork();

  switch (p) {
  case 0:
    // Llamamos rutina de manejo de operador
    run_pipe(comms, err);
    _exit(err->type);

  case -1:
    // Restauramos descriptores
    dup2(old_stdin, STDIN_FILENO);
    close(old_stdin);
    dup2(old_stdout, STDOUT_FILENO);
    close(old_stdout);

    debugf("Error al crear proceso hijo\n");
    err->type = ERR_DESCONOCIDO;

  default:
    // Restauramos descriptores
    dup2(old_stdin, STDIN_FILENO);
    close(old_stdin);
    dup2(old_stdout, STDOUT_FILENO);
    close(old_stdout);

    return p;
  }

  return 0;
}

pid_t run_pipe(Commands *comms, SpawnError *err) {
  assert(comms);
  assert(comms->count);
  assert(err);

  // Si hay N comandos en esta llamada compuesta, se construyen 2*(N-1) pipes
  int n_comms = comms->count;
  int n_pipes = n_comms - 1;

  // Se crea el arreglo de pipes y pids de procesos
  int *pipes = malloc((sizeof(int)) * 2 * n_pipes);
  pid_t *procs = malloc((sizeof(pid_t)) * n_comms);

  // Se construye cada pipe
  for (int i = 0; i < n_pipes; ++i) {
    if (pipe(&pipes[2 * i]) != 0) {
      err->type = ERR_PIPE;
      debugf("error pipes\n");
      return -1;
    }
  }

  // Se crea cada proceso
  for (int i = 0; i < n_comms; ++i) {
    // Obtenemos redirecciones en pipes
    int read_fd = -1, write_fd = -1;
    // Si no es el primero, se lee de la pipeline
    if (i > 0) {
      read_fd = pipes[(i - 1) * 2];
    }
    // Si no es el ultimo, se escribe en la pipeline
    if (i < n_comms - 1) {
      write_fd = pipes[2 * i + 1];
    }

    // Se ejecuta el proceso
    NodeType type = comms->c[i]->type;
    if (type == TySimpleCall) {
      SimpleCall *call = &comms->c[i]->value.scall;
      // Se crea el proceso
      procs[i] =
          exec_scall(call->program_name, call->params, read_fd, write_fd, err);

      // Si hay error al crear proceso, se sale
      if (procs[i] == 0) {
        return -1;
      }
    } else {
    }
  }

  // Hacemos wait sobre los procesos hijos y manejamos errores
  for (int i = 0; i < n_comms; ++i) {
    int status_code;
    waitpid(procs[i], &status_code, 0);

    NodeType type = comms->c[i]->type;
    if (type == TySimpleCall) {
      SimpleCall *call = &comms->c[i]->value.scall;

      // Procesamos status_code
      int decoded = WEXITSTATUS(status_code);
      int error_exec =
          (decoded == ERR_NOEXISTE_EXEC) || (decoded == ERR_ACCESO_EXEC);

      if (WIFEXITED(status_code) && !error_exec) {
        // Si el hijo termino normalmente y el exec se ejecuto sin errores
        debugf("Hijo termino bien y no hubo error de ejecucion\n");
        continue;
      } else if (WIFEXITED(status_code) && error_exec) {
        // Si el hijo termino normalmente pero el exec fallo
        // (Esto asume que el programa a ejecutar no usa los codigos de errores
        // 126 y 127. No deberian!)
        debugf("Hubo error de ejecucion\n");
        err->type = decoded;
        err->str = call->program_name;
        return -1;
      } else {
        // Si el hijo no termino normalmente, pero no tiene que ver con el
        // ejecutable (?)
        err->type = ERR_DESCONOCIDO;
        return -1;
      }
    } else {
      // Recursion :)
    }
  }

  debugf("Saliendo normalmente\n");
  return 0;
}