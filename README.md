Nombres: Alonso Maurel - Monserrath Morales.

  * BREVE EXPLICACION DE LO QUE SE SOLICITA:
Se solicita la creación de una aplicación que procese libros extraídos de archivos de texto. Los libros deben ser escaneados para obtener datos importantes como el título, la identificación (ID), la cantidad de caracteres y palabras, y otros detalles relevantes. La aplicación debe ofrecer varias funciones clave, incluyendo: Cargar documentos, mostrar documentos ordenados, buscar libro por título, palabras con mayor frecuencia y relevancia, mostrar contexto de una palabra.


  * IMPLEMENTACION CENTRAL:
Para implementar estas funciones, se utilizan tres estructuras de datos principales:
- mapa_libros: Almacena todos los libros escaneados de los archivos, lo que permite una gestión eficiente de los datos de los libros.
  
- id_libros: Almacena los libros, pero utiliza la ID como clave para acceder a ellos rápidamente. Esto es útil cuando se necesita acceder a libros específicos por su identificación.
  
- mapa_conectores: Contiene palabras que son conectores, preposiciones o pronombres que pueden interferir con las funciones de la aplicación. Estas palabras se utilizan para filtrarlas en el procesamiento de texto.

Ademas, se utilizaron las siguientes structs:
Para representar los libros, se utiliza una estructura Libro que contiene información esencial, como el ID, el título, la cantidad de palabras, la cantidad de caracteres y un HashMap para el almacenamiento de palabras y su frecuencia.

      typedef struct Libro {
        char ID[50];
        char titulo[50];
        int cant_palabras;
        int cant_caracteres;
        HashMap *hashmap_palabras;
      } Libro;

      typedef struct Relevancia {
        Libro* libro;
        float relevancia;
      } Relevancia;


  * ERRORES Y DIFICULTADES EN EL CODIGO:

1)
La optimización del código es esencial, ya que el programa presenta dificultades al procesar archivos de gran tamaño, lo que podría deberse en gran parte a la elección de estructuras de datos, como los hashmaps, y a la cantidad de operaciones de búsqueda e inserción. Hasta el momento, el programa sigue funcionando correctamente con libros más pequeños, como "Peter Pan," que contiene 66,642 líneas de texto.

2)
Ingresar palabras con mayúsculas o minúsculas es posible, y el programa funcionará correctamente. Sin embargo, es importante destacar que, aunque hemos definido una función para transformar las palabras a minúsculas, no la hemos aplicado en la función que muestra el contexto. Por lo tanto, al utilizar esta función, es necesario ingresar la palabra respetando las mayúsculas y minúsculas para encontrarla en el archivo. Hemos tomado la decisión de no modificar directamente el archivo, ya que creímos que esto podría requerir recursos adicionales que podrían optimizarse en una sola función que se ejecuta cuando es necesario.

3)
Para mostrar el contexto de una palabra, decidimos no utilizar las funciones proporcionadas por el profesor, como ftell y fseek, debido a que encontramos su funcionamiento bastante complejo y tuvimos dificultades para comprenderlo en su totalidad y aplicarlo en el codigo.

4)
A pesar de haber definido e inicializado un mapa de conectores, no pudimos aplicarlo a la inserción de palabras en la función que llena el mapa de palabras. En ese momento, nos encontramos con dificultades significativas y, para optimizar el tiempo y finalizar otros detalles del código, decidimos posponer esta funcionalidad. Nuestra idea principal era aplicarlo utilizando una condición 'if' y una búsqueda en el mapa antes de ingresar una palabra al mismo, aunque no pudimos implementarla en ese momento.


  * PARTICIPACION POR INTEGRANTE:
Alonso:
- Cargar documento
- buscar_libros
- palabras_mas_frecuentes
- palabras_mas_relevantes
- Obtener_titulo_caracter_palabra
porcentaje asignado: 100%

Monserrath: 
- Cargar documento
- mostrar_ordenados
- buscar_libros
- mostrar_palabraContexto
- buscar_libros_relevancia
porcentaje asignado: 100%

Se trabajó en conjunto para el diseño de la aplicación, organización del esquema principal y realización de este en cooperativo. Personalmente nos costo mucho esta tarea, por eso los porcentajes asignados. :,)