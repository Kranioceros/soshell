# sosh
Proyecto de shell simple para Sistemas Operativos

---
## Uso
A continuación se dan instrucciones básicas de uso para sosh.

sosh intepreta cualquier línea que se ingresa como un _comando sosh_.

Los comandos sosh están compuestos generalmente por _strings_, secuencias de caracteres **sin incluir caracteres reservados**.

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
    * `{grep Sistemas Operativos 20[0..9][0..9]*}`

Existen actualmente dos comandos sosh:

1. _Llamada simple_
2. _Llamada compuesta_

### 1. Llamada simple
Una llamada simple consiste del nombre de un programa y opcionalmente 1 o más parámetros para su invocación. Tanto los parámetros como el nombre del programa deben ser _strings_ separados por espacios.

Ejemplos:
* `ls`
* `ls -a`
* `cd mi/directorio/local`
* `grep {patron con espacios} archivo.txt`

Además, es posible redireccionar la entrada o la salida estándar del programa invocado.
* Para redireccionar la entrada estándar, se **antepone** a la llamada el operador `<` seguido de un archivo.
* Para redireccionar la salida estándar, se **agrega después** de la llamada el operador `>`, seguido de un archivo.

Ejemplos:
* `<archivo.txt tac`
* `grep regex archivo1 >archivo.txt`
* `<f1 sed {comandos ...} >f2`

### 2. Llamada compuesta
Una llamada compuesta consiste de varias llamadas simples unidas por algún operador. Este operador debe **ser el mismo** dentro de una misma llamada compuesta. Alternar operadores es un error sintáctico.

Actualmente solo hay un operador, el de pipe (`|`). Encadenando llamadas simples con pipes se construye una _tubería_. En una tubería, la sálida estándar de una llamada se convierte en la entrada estándar del siguiente.

Redireccionar la salida de la primer llamada de una tubería (mediante `>`), o la entrada de la última llamada de una tubería (mediante `<`), son errores semánticos. Sosh no va a ejecutar estas llamadas _mal formadas_, ya que no tienen un significado válido (¿qué significa redireccionar la entrada/salida de un programa dos veces?).

Ejemplos de tuberías:
* `grep patron archivo1 | wc -l`
* `<f1 cat | tac | tac >f2`

Ejemplos de tuberías mal formadas (las partes problemáticas marcadas con asteriscos):
* `grep patron {archivo 1.txt} **>f1** | wc -l`
* `grep patron {archivo 1.txt} | **<f1** wc -l`

---
## Compilación
Sosh requiere las siguientes herramientas para ser compilado:

* make (Se probó solo con GNU make)
* gcc
* flex
* bison
* libreadline-dev (Headers de desarrollo de libreadline7)

---
## Dependencias
Requiere las biblioteca dinámicas de flex y de readline7, pero estas deberían estar en prácticamente cualquier distribución Linux.