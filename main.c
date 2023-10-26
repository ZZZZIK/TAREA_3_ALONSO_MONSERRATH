#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "hashmap.h"
#include <math.h>

struct HashMap {
    Pair ** buckets;
    long size; //cantidad de datos/pairs en la tabla
    long capacity; //capacidad de la tabla
    long current; //indice del ultimo dato accedido
};

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


/* **1. Cargar documentos.** El usuario coloca los nombres de una lista de archivos txt separados por espacio y la aplicación los carga *(opcionalmente el usuario podría poner el nombre de una carpeta para cargar todo su contenido)*. */
void convertirMinusculas(char *palabra) {
    int i = 0;
    while (palabra[i]) {
        palabra[i] = tolower((unsigned char) palabra[i]);
        i++;
    }
}

void Obtener_titulo_caracter_palabra(FILE* archivo, Libro *libro){
  // contar caracteres, palabras y encontrar titulo
  char linea[1000];
  char *titulo = NULL;
  int contador_caracteres = 0;
  int contador_palabras = 0;
  int caracter_actual;
  bool en_palabra = false;

  while (fgets(linea, sizeof(linea), archivo) != NULL) {
    char *inicio = strstr(linea, "Title:");
    if (inicio != NULL) {
      char *fin_salto_linea = strchr(inicio, '\n');
      if (fin_salto_linea != NULL) {
        int longitud = fin_salto_linea - inicio - strlen("Title:");
        titulo = (char *)malloc((longitud + 1) * sizeof(char));
        strncpy(titulo, inicio + strlen("Title:"), longitud);
        titulo[longitud] = '\0';

        char *titulo_sin_espacios = titulo;
        while (isspace((unsigned char)*titulo_sin_espacios)) {
          titulo_sin_espacios++;
        }
        for (int i = strlen(titulo_sin_espacios) - 1; isspace((unsigned char)titulo_sin_espacios[i]); i--) {
          titulo_sin_espacios[i] = '\0';
        }

        // convertir título a minúsculas
        convertirMinusculas(titulo_sin_espacios);
        strcpy(libro->titulo, titulo_sin_espacios);
      }
    }

    // conteo de caracteres y palabras
    for (int i = 0; linea[i] != '\0'; i++) {
      char caracter_actual = linea[i];
      contador_caracteres++;

      if (caracter_actual == ' ' || caracter_actual == '\n' || caracter_actual == '\t') {
        en_palabra = false;
      } else {
        if (!en_palabra) {
          contador_palabras++;
          en_palabra = true;
        }
      }
    }

  }
  libro->cant_palabras = contador_palabras;
  libro->cant_caracteres = contador_caracteres;
  if (titulo == NULL) {
    printf("No se encontró el título en el archivo.\n");
    //exit(EXIT_FAILURE);
  } 
}

void cargar_libro(HashMap * mapa_libros, HashMap * ids_libros, HashMap * hashmap_conectores){
  // ------- LEER ENTRADA -------
  char entrada[1000];
  printf("\nIngresar nombres separados por espacio: ");
  int cont_archivos = 0;
  fgets(entrada, sizeof(entrada), stdin);
  entrada[strcspn(entrada, "\n")] = 0;
  char *palabra = NULL;
  char *IDS[100];

  palabra = strtok(entrada, " ");
  int i = 0;
  while (palabra != NULL) {
    IDS[i] = palabra;
    cont_archivos++;
    palabra = strtok(NULL, " ");
    i++;
  }
  // ------- ALMACENAR LIBROS EN MAPA -------
  for(int i = 0; i < cont_archivos;i++) {
    Libro * libro = (Libro *) malloc(sizeof(Libro));

    FILE *archivo = fopen(IDS[i], "r");
    if (archivo == NULL) {
      printf("No se encontró el archivo %s.\n", IDS[i]);
      free(libro);
      continue;
    }
    
    long capacity = 10000;
    HashMap* hash_mapa = createMap(capacity);

    char palabra_[100];
    while (fscanf(archivo, "%s", palabra_) != EOF) {
      // palabra a minuscula antes de almacenar en el mapa y procesar
      convertirMinusculas(palabra_);
      
      /*
      Pair *connector_pair = searchMap(hashmap_conectores, palabra_);
      if (connector_pair != NULL && strcmp(connector_pair->key, palabra_) == 0) {
          printf("|%s|",palabra_);
          printf("|%s|",connector_pair->key);
          continue; // Es un conector, omítelo
      }
      */
      
      int largo_palabra = strlen(palabra_);
      char ultimo_caracter = palabra_[largo_palabra - 1];

      if (ultimo_caracter == '.' || ultimo_caracter == ',') {
        palabra_[largo_palabra - 1] = '\0';
      }
      
      Pair* pair = searchMap(hash_mapa, palabra_);
      if (pair == NULL) {
          int* count = malloc(sizeof(int));
          *count = 1;
          insertMap(hash_mapa, strdup(palabra_), count);
          //printf("\n%s", palabra_);
      } else {
          int* count = (int*) pair->value;
          (*count)++;
          //printf("\n%s + 1", palabra_);
      }

      // si el mapa está casi lleno se amplia
      if (sizeMap(hash_mapa) > 0.75 * capacity) {
          enlarge(hash_mapa);
          capacity *= 2;
      }
    }
    rewind(archivo);

    // completar struct
    Obtener_titulo_caracter_palabra(archivo,libro);
    strcpy(libro->ID, IDS[i]);
    libro->hashmap_palabras = hash_mapa;
      
    printf("Titulo: %s\n", libro->titulo);
    printf("id: %s\n", libro->ID);
    printf("palabras: %d\n", libro->cant_palabras);
    printf("caracteres: %d\n", libro->cant_caracteres);
      
    insertMap(mapa_libros, libro->titulo, libro);
    insertMap(ids_libros, libro->ID, libro);
    
    fclose(archivo);
    palabra = strtok(NULL, " ");
  }
}


/* **2. Mostrar documentos ordenados.** La aplicación muestra los id y títulos de los libros ordenados por título. Además se muestra la **cantidad de palabras y caracteres** que tiene cada libro. */

int comparar_libros_por_titulo(const void *libro1, const void *libro2) {
    Libro *libroPtr1 = *(Libro**)libro1;
    Libro *libroPtr2 = *(Libro**)libro2;
    return strcmp(libroPtr1->titulo, libroPtr2->titulo);
}

void mostrar_ordenados(HashMap *mapa_libros) {
    int num_libros = sizeMap(mapa_libros);
    Libro **libros_ordenados = (Libro **)malloc(num_libros * sizeof(Libro *));
    int index = 0;

    Pair *current_pair = firstMap(mapa_libros);
    while (current_pair != NULL) {
        libros_ordenados[index] = (Libro *)current_pair->value;
        current_pair = nextMap(mapa_libros);
        index++;
    }

    // ordena el arreglo de libros por título
    qsort(libros_ordenados, num_libros, sizeof(Libro *), comparar_libros_por_titulo);

    printf("\nLibros ordenados por título:\n");
    for (int i = 0; i < num_libros; i++) {
        Libro *libro = libros_ordenados[i];
        printf("ID: %s\n", libro->ID);
        printf("Título: %s\n", libro->titulo);
        printf("Palabras: %d\n", libro->cant_palabras);
        printf("Caracteres: %d\n", libro->cant_caracteres);
        printf("\n");
    }
    free(libros_ordenados);
}


/* **3. Buscar un libro por título**. El usuario coloca algunas palabras separadas por espacio y la aplicación muestra los **títulos de libros** que contienen **todas las palabras**. */

void buscar_libros(HashMap * mapa_libros){
  // ------- LEER ENTRADA -------
  char entrada[1000];
  printf("\nIngresar palabras separadas por espacio: ");
  int cont_palabras = 0;
  fgets(entrada, sizeof(entrada), stdin);
  entrada[strcspn(entrada, "\n")] = 0;
  char *palabra = NULL;
  char *PALABRAS[100];

  palabra = strtok(entrada, " ");
  int i = 0;
  while (palabra != NULL) {
    PALABRAS[i] = palabra;
    cont_palabras++;
    convertirMinusculas(PALABRAS[i]); // palabra a minúsculas
    palabra = strtok(NULL, " ");
    i++;
  }

  printf("\nLibros que contienen todas las palabras:\n");
  int encontrado = 0;
  Pair* par;
  for (par = firstMap(mapa_libros); par != NULL; par = nextMap(mapa_libros)) {
    Libro* libro = (Libro*) par->value;
    int todasLasPalabrasEncontradas = 1;

    for (int j = 0; j < cont_palabras; j++) {
        char* palabra = PALABRAS[j];
        if (searchMap(libro->hashmap_palabras, palabra) == NULL) {
            todasLasPalabrasEncontradas = 0;
            break;
        }
    }
    if (todasLasPalabrasEncontradas) {
        printf("- %s.\n", libro->titulo);
        encontrado = 1;
    }
  }
  if (!encontrado) {
    printf("**** Ninguno ****\n");
  }

}

/*
**4. Palabras con mayor frecuencia.** El usuario ingresa el id de un libro y la aplicación muestra las 10 palabras que se repiten con mayor frecuencia (indicando la cantidad de veces que aparece cada una de ellas). *(La frecuencia se calcula como la cantidad de veces que aparece una palabra dividida por el total de palabras en el libro.)*
*/

void palabras_mas_frecuentes(HashMap *ids_libros) {
    char id[100];
    printf("Ingrese ID del libro: \n");
    scanf("%s", id);

    if (searchMap(ids_libros, id) == NULL) {
        printf("El libro no ha sido cargado \n");
        return;
    }

    Pair *Datos_Libro_buscado = searchMap(ids_libros, id);
    Libro *Libro_Buscado = Datos_Libro_buscado->value;
    HashMap *hash_mapa_palabras = Libro_Buscado->hashmap_palabras;

    Pair *diez_mayores[10] = {0};

    for (Pair *pair = firstMap(hash_mapa_palabras); pair != NULL; pair = nextMap(hash_mapa_palabras)) {
      //printf("|%s| =  |%d|\n", (char *)pair->key, *(int *)pair->value);
      char *palabra = (char *)pair->key;
      int repeticiones = *(int *)pair->value;
        
      int i;
      for (i = 0; i < 10; i++) {
        if (diez_mayores[i] == 0 || repeticiones > *((int *)diez_mayores[i]->value)) {
          break;
        }
      }

      for (int j = 9; j > i; j--) {
        diez_mayores[j] = diez_mayores[j-1];
      }

      diez_mayores[i] = pair;
    }
    printf("\n            TOP PALABRAS MAS REPETIDAS:\n\n");
    for (int i = 0; i < 10; i++) {
        if (diez_mayores[i] != NULL) {
          char* palabra_mayor_i = (char *)diez_mayores[i]->key;
          int repeticiones_palabra= *((int *)diez_mayores[i]->value);
          printf("POSICION [%d] %s = %d\n", i+1, palabra_mayor_i, repeticiones_palabra);
          free(palabra_mayor_i);
        }
    }
}

/* 
**5. Palabras más relevantes.**  El usuario ingresa el título de un libro y la aplicación muestra las 10 palabras **más relevantes** de este. Para calcular la relevancia de una palabra $p$ en un documento $d$ se debe aplicar la siguiente fórmula */

void palabras_mas_relevantes(HashMap *mapa_libros) {
    char titulo[100];
    printf("Ingrese el titulo del libro: \n");
    fgets(titulo, sizeof(titulo), stdin);
    titulo[strcspn(titulo, "\n")] = 0;
    printf("%s\n", titulo);

    if (searchMap(mapa_libros, titulo) == NULL) {
        printf("El libro no ha sido cargado \n");
        return;
    }

    Pair *Datos_Libro_buscado = searchMap(mapa_libros, titulo);
    Libro *Libro_Buscado = Datos_Libro_buscado->value;
    HashMap *hash_mapa_palabras = Libro_Buscado->hashmap_palabras;
    int cant_total_palabras = Libro_Buscado->cant_palabras;

    Pair *diez_mayores[10] = {NULL};

    for (Pair *pair = firstMap(hash_mapa_palabras); pair != NULL; pair = nextMap(hash_mapa_palabras)) {
        char *palabra = (char *)pair->key;
        int repeticiones = *(int *)pair->value;
        float relevancia = ((float)repeticiones / cant_total_palabras);

        int i;
        for (i = 0; i < 10; i++) {
            if (diez_mayores[i] == NULL || relevancia > *((float *)diez_mayores[i]->value)) {
                break;
            }
        }

        for (int j = 9; j > i; j--) {
            diez_mayores[j] = diez_mayores[j - 1];
        }

        Pair *par = malloc(sizeof(Pair));
        par->key = pair->key;
        par->value = malloc(sizeof(float));
        *((float *)par->value) = relevancia;
        diez_mayores[i] = par;
    }

    printf("\n            TOP PALABRAS CON MAYOR RELEVANCIA :\n\n");
    for (int i = 0; i < 10; i++) {
        if (diez_mayores[i] != NULL) {
            float relevancia_palabra = *((float *)diez_mayores[i]->value) * 100; 
            printf("POSICION [%d] %s = %.2f%%\n", i + 1, (char *)diez_mayores[i]->key, relevancia_palabra);
            free(diez_mayores[i]->key);
            free(diez_mayores[i]->value);
            free(diez_mayores[i]);
        }
    }
}


/* **6. Buscar por palabra.** El usuario ingresa una palabra y la aplicación muestra los libros (id y título) que tienen la palabra en su **contenido**. Los libros deben ser ordenados por la relevancia de la palabra buscada.  *Por ejemplo, si busco “Jesús”, la biblia debería aparecer en primer lugar.* */
int compararRelevancia(const void *a, const void *b) {
    const Relevancia *relevanciaA = (const Relevancia *)a;
    const Relevancia *relevanciaB = (const Relevancia *)b;
    if (relevanciaA->relevancia < relevanciaB->relevancia) return 1;
    if (relevanciaA->relevancia > relevanciaB->relevancia) return -1;
    return 0;
}

void buscar_libros_relevancia(HashMap * mapa_libros){
  char palabra[100];
  printf("Ingrese palabra: ");
  scanf("%s", palabra);

  int numLibros = sizeMap(mapa_libros);
  Relevancia relevanciaLibros[numLibros];
  int index = 0;

  for (Pair *pair = firstMap(mapa_libros); pair != NULL; pair = nextMap(mapa_libros)) {
      Libro *libro = (Libro *)pair->value;
      HashMap *hashmap = libro->hashmap_palabras;
      Pair *palabraPair = searchMap(hashmap, palabra);
      int cant_total_palabras=libro->cant_palabras;
    
      if (palabraPair != NULL) {
          int repeticiones = *(int *)palabraPair->value;
          float relevancia = ((float)repeticiones / cant_total_palabras);
              
          relevanciaLibros[index].libro = libro;
          relevanciaLibros[index].relevancia = relevancia;
          index++;
      }
  }
  // ordenar lista de libros X relevancia (descendente)
  qsort(relevanciaLibros, numLibros, sizeof(Relevancia), compararRelevancia);

  printf("\nLibros que contienen la palabra '%s' ordenados por relevancia:\n", palabra);
  for (int i = 0; i < index; i++) {
      Libro *libro = relevanciaLibros[i].libro;
      printf("ID: %s, Título: %s, Relevancia: %.2f\n", libro->ID, libro->titulo, relevanciaLibros[i].relevancia*100);
  }
}


/* **7. Mostrar palabra en su contexto dentro del libro.** El usuario ingresa el título de un libro y el de una palabra a buscar. La aplicación muestra las distintas apariciones de la palabra **en el contexto del documento**, es decir, para cada aparición, se muestran algunas palabras hacia atrás y hacia adelante de la palabra buscada (por ejemplo, la línea completa en la que aparece la palabra, o 5 palabras hacia atrás y 5 hacia adelante). */
void mostrar_palabraContexto(HashMap * mapa_libros) {
  char titulo[100];
  char palabra[100];
  printf("\nIngrese titulo: ");
  fgets(titulo, sizeof(titulo), stdin);
  titulo[strcspn(titulo, "\n")] = 0;
  printf("Ingrese palabra: ");
  scanf("%s", palabra);
  
  int libroEncontrado = 0;
  for (int i = 0; i <mapa_libros->capacity; i++) {
    Pair* pair = mapa_libros->buckets[i];
    if (pair != NULL) {
      Libro* libro = (Libro*) pair->value;
      if (strcmp(libro->titulo, titulo) == 0) {
        libroEncontrado = 1;
        FILE* ptr = fopen(libro->ID, "r");
        if (ptr == NULL) {
          printf("\nNo se pudo abrir el archivo del libro");
          return;
        }
        char linea[256];
        while (fgets(linea, sizeof(linea), ptr) != NULL) {
          if (strstr(linea, palabra) != NULL) {
            printf("%s", linea);
          }
        }
        
        fclose(ptr);
        return;
      }
    }
  }
  if (libroEncontrado == 0) {
    printf("\nNo se encontró el libro con el título ingresado");
  }
}

// ------- FUNCIONES EXTRAS ---------
// MAPA CONECTORES
void crear_mapaConectores(HashMap *mapa) {
  insertMap(mapa, "he", NULL);
  insertMap(mapa, "she", NULL);
  insertMap(mapa, "it", NULL);
  insertMap(mapa, "we", NULL);
  insertMap(mapa, "they", NULL);
  insertMap(mapa, "you", NULL);
  insertMap(mapa, "i", NULL);
  insertMap(mapa, "are", NULL);
  insertMap(mapa, "is", NULL);
  insertMap(mapa, "as", NULL);
  insertMap(mapa, "a", NULL);
  insertMap(mapa, "so", NULL);
  insertMap(mapa, "on", NULL);
  insertMap(mapa, "in", NULL);
  insertMap(mapa, "at", NULL);
  insertMap(mapa, "to", NULL);
  insertMap(mapa, "the", NULL);
  insertMap(mapa, "that", NULL);
  insertMap(mapa, "those", NULL);
  insertMap(mapa, "there", NULL);
  insertMap(mapa, "and", NULL);
  insertMap(mapa, "also", NULL);
  insertMap(mapa, "additionally", NULL);
  insertMap(mapa, "firstly", NULL);
  insertMap(mapa, "furthermore", NULL);
  insertMap(mapa, "moreover", NULL);
  insertMap(mapa, "in addition", NULL);
  insertMap(mapa, "besides", NULL);
  insertMap(mapa, "likewise", NULL);
  insertMap(mapa, "plus", NULL);
  insertMap(mapa, "but", NULL);
  insertMap(mapa, "however", NULL);
  insertMap(mapa, "yet", NULL);
  insertMap(mapa, "although", NULL);
  insertMap(mapa, "even", NULL);
  insertMap(mapa, "though", NULL);
  insertMap(mapa, "nevertheless", NULL);
  insertMap(mapa, "other", NULL);
  insertMap(mapa, "while", NULL);
  insertMap(mapa, "contrast", NULL);
  insertMap(mapa, "nonetheless", NULL);
  insertMap(mapa, "because", NULL);
  insertMap(mapa, "since", NULL);
  insertMap(mapa, "as", NULL);
  insertMap(mapa, "consequently", NULL);
  insertMap(mapa, "therefore", NULL);
  insertMap(mapa, "thus", NULL);
  insertMap(mapa, "hence", NULL);
  insertMap(mapa, "due", NULL);
  insertMap(mapa, "owing to", NULL);
  insertMap(mapa, "if", NULL);
  insertMap(mapa, "unless", NULL);
  insertMap(mapa, "supposing", NULL);
  insertMap(mapa, "second", NULL);
  insertMap(mapa, "next", NULL);
  insertMap(mapa, "then", NULL);
  insertMap(mapa, "finally", NULL);
  insertMap(mapa, "meanwhile", NULL);
  insertMap(mapa, "subsequently", NULL);
  insertMap(mapa, "meantime", NULL);
  insertMap(mapa, "overall", NULL);
  insertMap(mapa, "therefore", NULL);
  
}

int main() {
  HashMap * mapa_libros = createMap(100); // clave: titulo
  HashMap * ids_libros = createMap(100); // clave: ids
  HashMap * mapa_conectores = createMap(100);
  crear_mapaConectores(mapa_conectores);
  
  while (1) {
    int opcion;
    printf("\n---------------------------------------");
    printf("\n            MENU DE OPCIONES");
    printf("\n1. Cargar documentos");
    printf("\n2. Mostrar documentos ordenados");
    printf("\n3. Buscar libro por título");
    printf("\n4. Palabras con mayor frecuencia de un libro");
    printf("\n5. Palabras más relevantes de un libro");
    printf("\n6. Buscar libros por palabra");
    printf("\n7. Mostrar contexto de una palabra");
    printf("\n8. Salir del programa\n");
    scanf("%d", &opcion);
    printf("---------------------------------------\n");

    if (opcion==1){
      getchar();
      cargar_libro(mapa_libros,ids_libros,mapa_conectores);
    }
    else if (opcion==2){
      getchar();
      mostrar_ordenados(mapa_libros);
    }
    else if(opcion==3){
      getchar();
      buscar_libros(mapa_libros);
    }
    else if(opcion==4){
      getchar();
      palabras_mas_frecuentes(ids_libros);
    }
    else if(opcion==5){
      getchar();
      palabras_mas_relevantes(mapa_libros);
    }
    
    else if(opcion==6){
      getchar();
      buscar_libros_relevancia(mapa_libros);
    } 
    else if(opcion==7){
      getchar();
      mostrar_palabraContexto(mapa_libros);
    }
    else if(opcion==8){
      return 0;
    }
  }
}