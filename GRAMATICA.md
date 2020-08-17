Gramática
=========

En este documento definimos la gramática que utilizamos para el lenguaje sosh.

Aqui definimos mas que nada el significado de cada simbolo. Las reglas de produccion de los mismos se encuentran en los archivos de flex y sosh listados al final de este documento.

# Símbolos terminales
En las reglas de producción, se presentan en MAYÚSCULAS.

* **SIMPLE_STRING**:
    * Cualquier secuencia de caracteres, sin incluir caracteres reservados ni espacios.
* **BEGIN_DELIM_STRING**:
    * Secuencia de caracteres delimitada, puede incluir espacios y caracteres reservados, con la excepción de las llaves (`{`,`}`) y la nueva línea (`\n`). Inicia con llave abierta (`{`).
* **`}`**:
    * Llave cerrada (`}`), utilizada para terminar un *delim_string*.
* **`$`**:
    * Símbolo dólar (`$`), utilizado para evaluación de variables. *(No usado)*
* **`=`**:
    * Símbolo igual (`=`), utilizado para definir variables. *(No usado)*
* **`|`**:
    * Símbolo de barra vertical (`|`), utilizado para conectar procesos mediante pipes.
* **`>`**:
    * Símbolo de mayor (`>`), utilizado para redirigir la salida estándar de un proceso hacia un archivo.
* **`<`**
    * Símbolo de menor (`<`), utilizado para redirigir un archivo hacia la entrada estándar de un proceso.
* **EOL**:
    * Símbolo de nueva línea, utilizado para terminar el comando.

## Tipos de caracteres:

* Caracteres reservados:
    1. Llave abierta (`{`) y llave cerrada (`}`).
    2. Corchete abierto (`[`) y corchete cerrado (`]`).
    3. Símbolo dólar (`$`).
    4. Símbolo igual (`=`).
    5. Símbolo barra vertical (`|`).
    6. Símbolo mayor y menor (`>`,`<`).

* Espacios:
    1. Espacio simple
    2. Tabulación
    3. Nueva línea

# Símbolos no terminales
En las reglas de producción se presentan en minúsculas.

* *sosh*:
    * Símbolo no terminal objetivo (goal). Representa un comando ingresado en la terminal, termina siempre en **EOL**.
* *call*:
    * Representa una llamada simple a un único programa  o una compuesta a varios programas relacionados por operadores. Puede ser *simple_call* o *compound_call*.
* *compound_call*:
    * Representa una llamada a varios programas relacionados por único operador.
* *simple_call*:
    * Representa una llamada a un único programa, con posible redirección de stdin o stdout. Opcionalmente, puede estar
    rodeada por corchetes (`[]`). Agrupar una unica llamada simple no tiene sentido, pero tampoco tiene que ser un error. Una *simple_call* es una *ungrouped_simple_call*.
* *ungrouped_simple_call*:
    * Representa una llamada generica a un unico programa. Puede ser con redireccion de entrada, salida, ambas o ninguna.
* *single_call*:
    * Llamada a un único programa, con 0 o más parámetros. Incluye redireccion de ningun tipo.
* *call_from_to_file*:
    * Llamada a un único programa, con 0 o más parámetros, con redirección de stdout y stdin a archivos.
* *call_to_file*:
    * Llamada a un único programa, con 0 o más parámetros, con redirección de stdout a un archivo.
* *call_from_file*:
    * Llamada a un único programa, con 0 o más parámetros, con redirección de stdin a un archivo.
* *params*:
    * Representa un conjunto de 1 o más *string*, los parámetros del programa llamado.
* *string*:
    * Representa un string cualquiera. Puede tratarse de un **SIMPLE_STRING** o un *delim_string*.
* *delim_string*:
    * Representa un string que puede contener espacios y/o algunos caracteres reservados. Consiste de un **BEGIN_DELIM_STRING** seguido de un **END_DELIM_STRING**.

# Reglas de producción

Para ver las reglas de produccion de cada simbolo no terminal, leer el archivo de bison `parser/sosh.y`.

Para ver las reglas de produccion de cada simbol terminal, leer el archivo de flex `parser/sosh.l`.