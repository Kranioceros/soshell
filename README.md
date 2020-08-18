# sosh
Proyecto de shell simple para Sistemas Operativos

Disponible en [github.com/Kranioceros/soshell](https://github.com/Kranioceros/soshell).

## Uso
A continuación se dan instrucciones básicas de uso para sosh.

sosh intepreta cualquier línea que se ingresa como un _comando sosh_.

Los comandos sosh están compuestos generalmente por _strings_, secuencias de caracteres **sin incluir caracteres reservados**.

Para una lista de los carácteres reservados y otros símbolos, leer el documento acompañante con la gramática del lenguaje.

Existen actualmente dos tipos de strings:
* _Strings sencillos_:\
Secuencias simples de caracteres sin incluir caracteres reservados ni espacios.\
Ejemplos:
    * `ls`
    * `/home/sosh/archivo1`
    * `-opcion1`
* _Strings delimitados_:\
Secuencias simples de caracteres delimitadas por llaves. Estos sí pueden incluir espacios o caracteres reservados, con la excepción de las llaves.\
Ejemplos:
    * `{Archivo con espacios.txt}`
    * `{20[0..9][0..9]*}`

Existen actualmente dos comandos sosh:

1. _Llamada simple_
2. _Llamada compuesta_

### 1. Llamada simple

### Parámetros

Una llamada simple consiste del nombre de un programa y opcionalmente 1 o más parámetros para su invocación. Tanto los parámetros como el nombre del programa deben ser _strings_ separados por espacios.

Ejemplos:
* `ls`
* `ls -a`
* `cd mi/directorio/local`
* `grep {patron con espacios} archivo.txt`

Es importante destacar que en un *string delimitado*, los espacios **pierden su significado como separadores**. Esto es consistente con otros shells (como el Bourne shell disponible por defecto en la mayoría de las distribuciones Linux).

Por lo tanto, el siguiente comando contiene un error:

* `grep {1[0-9]* /etc/passwd}`

El usuario probablemente quiso pasar dos parámetros separados a grep: `1[0-9]*` y `/etc/passwd`. Pero debido a que utilizó un único string delimitado, `grep` solo recibirirá un parámetro con el contenido del string completo.

### Redirección de entrada/salida estándar

Además, es posible redireccionar la entrada o la salida estándar del programa invocado.
* Para redireccionar la entrada estándar, se **antepone** a la llamada el operador `<` seguido de un archivo.
* Para redireccionar la salida estándar, se **agrega después** de la llamada el operador `>`, seguido de un archivo.

Ejemplos:
* `<archivo.txt tac`
* `grep regex archivo1 >archivo.txt`
* `<f1 sed {comandos sed...} >f2`

### 2. Llamada compuesta
Una llamada compuesta consiste de varias llamadas simples unidas por algún operador. Este operador debe **ser el mismo** dentro de una misma llamada compuesta. Alternar operadores es un error sintáctico.

Actualmente solo hay un operador, el de pipe (`|`). Encadenando llamadas simples con pipes se construye una _tubería_. En una tubería, la sálida estándar de una llamada se convierte en la entrada estándar de la siguiente.

Redireccionar la salida de la primer llamada de una tubería (mediante `>`), o la entrada de la última llamada de una tubería (mediante `<`), son errores semánticos. Sosh no va a ejecutar estas llamadas _mal formadas_, ya que no tienen un significado válido (¿qué significa redireccionar la entrada/salida de un programa dos veces?).[^1]

Ejemplos de tuberías:
* `grep patron archivo1 | wc -l`
* `<f1 cat | tac | tac >f2`

Ejemplos de tuberías mal formadas (las partes problemáticas marcadas con asteriscos):
* `grep patron {archivo 1.txt} **>f1** | wc -l`
* `grep patron {archivo 1.txt} | **<f1** wc -l`


## Compilación
Sosh requiere las siguientes herramientas para ser compilado:

* make (Se probó solo con GNU make)
* gcc (Se probó con versión 10.2.0)
* flex
* bison
* libreadline-dev (Headers de desarrollo de libreadline)

Para compilar, solo inicie una terminal dentro de la carpeta del proyecto y ejecute `make`. El ejecutable final se debería encontrar en la carpeta `bin`.

Para una versión de debug con mensajes de las funciones llamadas e información sobre el parser
y el AST construido, ejecute `make debug`. El ejecutable debug debería estar en la carpeta `debug`.

**Nota:** Debido a las dificultades de escribir un Makefile de la forma correcta (un arte oculta), éste no funciona del todo bien para reconocer las dependencias del proyecto a actualizar.

Ante cualquier problema de compilación, ejecute un `make clean` para borrar todos los archivos objeto e intermedios creados hasta el momento.


## Dependencias
Requiere las bibliotecas dinámicas de flex y de readline (que proveen las banderas de linkeo `-lfl` y `-lreadline`), pero estas deberían estar en prácticamente cualquier distribución Linux.


## Limitaciones

* Actualmente el scanner utilizado por sosh es muy primitivo y **no contiene soporte para secuencias de escape**. Esto significa que, debido al uso de las llaves (`{}`) como caracteres delimitadores en strings delimitados, **es imposible actualmente usar las llaves dentro de un string**.

* El parser de sosh también es muy primitivo y no guarda información de errores cuando ocurren, lo cual implica que si hay errores sintácticos, **es incapaz de proveer mensajes de ayuda que señalen la ubicación o el tipo de error**.

* El parser de sosh construye el árbol sintáctico (AST) a medida que reconoce los símbolos que definen su gramática. Si bien se tuvo el cuidado de no introducir llamadas dobles a `free` que puedan terminar el programa abruptamente cuando se elimina el árbol de memoria, por cuestiones de tiempo, **cuando hay un error sintáctico no se elimina el árbol de memoria**, lo cual introduce una fuga de memoria en el programa.

* La implementación de tuberías utiliza un proceso hijo que se desprende del proceso principal, que a su vez maneja a todas las llamadas contenidas en la tubería.\
\
Proceso principal -> Proceso manejador de tubería -> Llamada 1, Llamada 2, ...\
\
Esto complica la comunicación de errores de las llamada contenidas en las tuberías al proceso principal (que es el que reporta los errores).\
\
Debido a esta decisión de diseño, resulta imposible comunicar los errores completos al proceso principal sin usar algún método de comunicación entre procesos. Esto no se hizo por cuestiones de tiempo, así que el proceso principal puede indicar que hubo un error en la tubería **pero no indicar exactamente cual y en qué llamada de la misma**.

[1]: Existe una interpretación válida para el caso de la redirección de salida estándar en cualquier llamada dentro de una tubería (salvo la última). Podría *dividirse* la salida de la llamada redireccionada, yendo su contenido tanto a la siguiente llamada como a un archivo.

Pero esta interpretación sería bastante más compleja de implementar, su uso no sería habitual y probablemente termine permitiendo errores inadvertidos por parte del usuario.