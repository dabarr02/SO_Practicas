#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "defs.h"
#include <semaphore.h>
#define M 5

int finish = 0;
share_cocina *ptr; //memoria compartida
sem_t *sem_cocinero; //semaforo
sem_t *sem_salvajes; //semaforo


void putServingsInPot(int servings)
{
	
	ptr->nr_servings+=servings; //recargamos los servings
	printf("[Cocinero] Comida servida\n");
    printf("[Cocinero] Comida actual: %d\n",ptr->nr_servings);
    return;
}

void cook(void)
{
	while(!finish) {
		sem_wait(sem_cocinero); // esperar a que los salvajes notifiquen
        if (!finish) {
            printf("[Cocinero] Cocinero despierto\n");
            putServingsInPot(M);
            printf("[Cocinero] Despertando salvajes\n");
            sem_post(sem_salvajes); // liberar a los salvajes
        }
    }
    return;
}
void handler(int signo)
{
	
	 finish = 1;
    sem_post(sem_cocinero); // liberar al cocinero si está esperando

}

int main(int argc, char *argv[])
{
	
//--Aqui manejamos la gestión de las señales para parar el programa.----------------------------------

	struct sigaction sa; //estructura de la accción 
    sa.sa_handler = handler; //Esta es la funcion que se ejecutara con sigalarm
    sa.sa_flags = 0;

	if (sigaction(SIGTERM, &sa, NULL) == -1) { //cambiamos la accion de SIGALARM
        perror("sigaction SIGTERM ");
        exit(EXIT_FAILURE);
    }
	if (sigaction(SIGINT, &sa, NULL) == -1) { //cambiamos la accion de SIGALARM
        perror("sigaction SIGINT ");
        exit(EXIT_FAILURE);
    }

	//-----------Fin Gestion de señales----------

	//----Creación de recursos compartidos-------
	const char *name= "/cocina"; //nombre de la memoria compartida
	const size_t size= sizeof(share_cocina); //tamaño que le vamos a dar
	const char *sem_name_cook = "/sem_cocinero"; // nombre del semáforo
    const char *sem_name_savg = "/sem_salvajes";

    // Eliminar semáforos y memoria compartida existentes para evitar conflictos de ejecuciones anteriores.
    sem_unlink(sem_name_cook);  // Eliminamos cualquier semáforo cocinero anterior.
    sem_unlink(sem_name_savg);  // Eliminamos cualquier semáforo de los salvajes anterior.
    shm_unlink(name);  // Eliminamos cualquier memoria compartida anterior.

	int shm_fd = shm_open(name,O_CREAT | O_RDWR, 0666); //creamos la memoria compartida con nombre name, creat para crear, rdwr para leer y escribir, 0666 permisos
	if (shm_fd == -1) { //si falla mandar error
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

	// Establecer el tamaño del objeto de memoria compartida !!!!!!!!!! Sin esto no funcinona, por que no has reservado el espacio!!!!!!!!!!!!!
    if (ftruncate(shm_fd, size) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

	ptr = mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0); //Null para que elija el compilador la zona de momoria, tamaño, flags para que se pueda leer y escribir,
	//shared para que todos vean los cambios, el descriptor de fichero de la memoria compartida,0 el offset
	if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
	
	// Crear el semáforo cocinero
    sem_cocinero = sem_open(sem_name_cook, O_CREAT, 0644,0);
    if (sem_cocinero == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    sem_salvajes = sem_open(sem_name_savg, O_CREAT, 0644, 1);// crear semaforo salvajes
    if (sem_salvajes == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

	ptr->cook_wait=0;
	ptr->nr_servings=0;
	ptr->sav_wait=0;
	
	cook();
	

    // Cerrar el descriptor de archivo
    if (close(shm_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    // Eliminar el objeto de memoria compartida
    if (shm_unlink(name) == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
    //Cerramos y desvinculamos cerrojos
    // Cerrar los semáforos
    if (sem_close(sem_cocinero) == -1) {
        perror("sem_close sem_cocinero");
        exit(EXIT_FAILURE);
    }

    if (sem_close(sem_salvajes) == -1) {
        perror("sem_close sem_salvajes");
        exit(EXIT_FAILURE);
    }

    // Eliminar los semáforos
    if (sem_unlink(sem_name_cook) == -1) {
        perror("sem_unlink sem_cocinero");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(sem_name_savg) == -1) {
        perror("sem_unlink sem_salvajes");
        exit(EXIT_FAILURE);
    }
    
     // Desmapear la memoria compartida
    if (munmap(ptr, size) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }


	return 0;
}
