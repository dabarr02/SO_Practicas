#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <unistd.h> 
#include <stdbool.h>
/*
	Acabado

*/
/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor
 *
 * The loadstr() function must allocate memory from the heap to store
 * the contents of the string read from the FILE.
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc())
 *
 * Returns: !=NULL if success, NULL if error
 */

char * progname;

char *loadstr(FILE *file, int *contar) {
    char c;
    *contar = 0;

    // Contar la longitud de la cadena hasta el carácter nulo
    while (fread(&c, sizeof(char), 1, file) == 1) {
        if (c == '\0') {
            break; // Se encontró el carácter nulo
        }
        (*contar)++;
    }

    // Si no hay caracteres leídos, retornar NULL
    if (*contar == 0) {
        return NULL;
    }

    // Asignar memoria para la cadena +1 para el carácter nulo
    char *str = (char *)malloc((*contar + 1) * sizeof(char));
    if (str == NULL) {
        perror("Error al asignar memoria");
        return NULL; // Manejo de error de asignación
    }

    // Regresar al inicio de la cadena leída
    fseek(file, -(*contar + 1), SEEK_CUR); // Mover el puntero para leer la cadena
    fread(str, sizeof(char), *contar, file); // Leer la cadena
	fseek(file, 1, SEEK_CUR);
    str[*contar] = '\0'; // Añadir carácter nulo al final

    return str;	
}


int main(int argc, char *argv[])
{
	/* To be completed */

	FILE* file=NULL;
	
	char* title=NULL;
	

	progname = argv[0];

	

	/* Parse command-line options */
	title=argv[1];
	printf("Abriendo: %s\n",title);
	file=fopen(title,"r");
	if (!file){
		perror("out:"); 
		exit(1);
	}
	int * contar =(int*)malloc(sizeof(int));
	char* str = loadstr(file,contar);
	char n='\n';
	while(str!=NULL){
		int ret=0;
		ret=fwrite(str,sizeof(char),*contar + 1,stdout);
		fwrite(&n,sizeof(char),1,stdout);
		//write(1, str, (*contar+1) * sizeof(char));
		if (ret==EOF){
			fclose(file);
			err(3,"fwrite) failed!!");
		}
		str=loadstr(file,contar);
	}
	fclose(file);
	free(str);
	free(contar);
	return 0;

}
