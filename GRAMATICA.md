Gramática
=========

En este documento definimos la gramática que emplearemos para sosh.

Primero definiremos los símbolos terminales y no terminales y su significado, antes de dar las reglas de producción en la forma BNF.

# Símbolos terminales
En las reglas de producción, se presentan en MAYÚSCULAS.

* **SIMPLE_STRING**:
    Cualquier secuencia de caracteres, sin incluir caracteres reservados ni espacios.
* **BEGIN_DELIM_STRING**:
    Secuencia de caracteres delimitada, puede incluir espacios y caracteres reservados, con la excepción de las llaves (`{`,`}`) y la nueva línea (`\n`). Inicia con llave abierta (`{`).
* **`}`**:
    Llave cerrada (`}`), utilizada para terminar un *delim_string*.
* **`$`**:
    Símbolo dólar (`$`), utilizado para evaluación de variables.
* **`=`**:
    Símbolo igual (`=`), utilizado para definir variables.
* **`|`**:
    Símbolo de barra vertical (`|`), utilizado para conectar procesos mediante pipes.
* **`>`**:
    Símbolo de mayor (`>`), utilizado para redirigir la salida estándar de un proceso hacia un archivo.
* **`<`**
    Símbolo de menor (`<`), utilizado para redirigir un archivo hacia la entrada estándar de un proceso.
* **EOL**:
    Símbolo de nueva línea, utilizado para terminar el comando.

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
    Símbolo no terminal objetivo (goal). Representa un comando ingresado en la terminal, termina siempre en **EOL**.
* *sosh_command*:
    Representa una llamada o una asignación de variable. Puede ser *call* o *assignation*
* *assignation*:
    Representa la asignación de un valor a una variable. POR DEFINIR.
* *call*:
    Representa una llamada simple a un único programa  o una compuesta a varios programas relacionados por operadores. Puede ser *simple_call* o *compound_call*.
* *compound_call*:
    Representa una llamada a varios programas relacionados por un operador. Puede ser *pipeline* o
    *sequence*.
* *pipeline*:
    Una llamada a varios programas, donde sus entradas y salidas se encuentran unidas por pipes.
    Consiste de un *pipe_start* y un *pipe_end*.
* *pipe_start*:
    Una llamada a un programa, con redirección de stdin o no. Pero **no** tiene redirección de stdout, ya que esto rompe la pipeline.
* *pipe_end*:
    Una sucesión de llamadas a 1 o más programas, donde ninguno tiene redirección de stdout, salvo el último, que puede tener o no.
* *sequence*:
    Representa una llamada a varios programas secuenciados, donde uno empieza cuando termina el anterior. POR DEFINIR.
* *simple_call*:
    Representa una llamada a un único programa, con posible redirección de stdin o stdout. Puede ser *single_call*, *call_to_file* o *call_from_to_file*
* *single_call*:
    Llamada a un único programa, con 0 o más parámetros.
* *call_from_to_file*:
    Llamada a un único programa, con 0 o más parámetros, con redirección de stdout a un archivo. Puede tener redireccion de stdin o no.
* *call_to_file*:
    Llamada a un único programa, con 0 o más parámetros, con redirección de stdout a un archivo.
* *call_from_file*:
    Llamada a un único programa, con 0 o más parámetros, con redirección de stdin a un archivo.
* *params*:
    Representa un conjunto de 1 o más *string*, los parámetros del programa llamado.
* *string*:
    Representa un string cualquiera. Puede tratarse de un **SIMPLE_STRING**, un *delim_string* o de un *eval_var*.
* *delim_string*:
    Representa un string que puede contener espacios o algunos caracteres reservados. Consiste de un **BEGIN_DELIM_STRING** seguido de un **END_DELIM_STRING**.
* *eval_var*:
    Representa la evaluación de una variable. Comienza siempre con **DOLLAR**, pero le puede seguir un **SIMPLE_STRING** o un *delim_string*.

# Reglas de producción
*sosh* ::= *sosh_command* **END**

*sosh_command* ::=\
&ensp;*call*\
&ensp;| *assignation*

*call* ::=\
&ensp;*simple_call*\
&ensp;| *compound_call*

*compound_call* ::=\
&ensp;*pipeline*\
&ensp;| *sequence*

*pipeline* ::=\
&ensp;*pipe_start* *pipe_end*

*pipe_start* ::=\
&ensp;*single_call* **`|`**\
&ensp;| *call_from_file* **`|`**

*pipe_end* ::=\
&ensp;*single_call*\
&ensp;| *call_to_file*\
&ensp;| *pipe_end* *single_call* `|`

*simple_call* ::=\
&ensp;*single_call*\
&ensp;| *call_from_file*\
&ensp;| *call_from_to_file*

*call_from_to_file* ::=\
&ensp;*call_to_file*\
&ensp;| *call_from_file* **`>`** *string*

*call_to_file* ::=\
&ensp;*single_call* **`>`** *string*

*call_from_file* ::=\
&ensp;**`<`** *string* *single_call

*single_call* ::=\
&ensp;*string*
&ensp;| *string* *params*\

*params* ::=\
&ensp;*string*
&ensp;| *params* *string*

*string* ::=\
&ensp;**SIMPLE_STRING**\
&ensp;| **BEGIN_DELIM_STRING** **END_DELIM_STRING**\
&ensp;| *eval_var*

// No se usa por ahora
*eval_var* ::=\
&ensp;**`$`** **SIMPLE_STRING**\
&ensp;| **`$`** **BEGIN_DELIM_STRING** **END_DELIM_STRING**