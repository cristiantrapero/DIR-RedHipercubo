//Simulacion de una red hipercubo
//Cristian Trapero Mora
//Compilar: mpicc -g hipercubo.c -o hipercubo -DL=dimension
//Ejecutar: mpirun -n 2^dimension hipercubo

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

//Lado de la red hipercubo
//#define L 3

//Nombre del fichero del cual vamos a leer
#define DATOSDAT "datos.dat"

//Numero maximo de numeros contenidos en el fichero datos.dat
#define MAX_FILE 1024
#define MAX_ITEMS 1024

//Metodo para leer el fichero de datos, usado por el rank 0
int leerFichero(double *numeros);

//Funcion para obtener los vecinos de un rank
void obtenerVecinos(int rank, int *vecinosAdyacentes);

//Funcion para obtener el numero maximo en toda la red hipercubo
double obtenerMaximo(int rank, double bufferNumero, int *vecinos);


int main(int argc, char *argv[]){

	int rank, size;
	MPI_Status status;

	//Buffer que almacena el numero de cada rank
	double bufferNumero;

	//Mayor numero de toda la red
	double numeroMaximo;

	//Indica el numero de procesos que se deben lanzar para una dimension dada
	int numeroProcesosALanzar=(int) round(pow(2,L));

	//Ranks de los vecinos correspondientes al rank
	int vecinos[L];

	//Variable seguir nos dira si seguir con la ejecución(1) o no(0). 
	int seguir=1;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);


	//SI SOY EL RANK 0
	if (rank==0){

		//Si no hemos lanzado el numero de procesos adecuado al hipercubo
		if(numeroProcesosALanzar!=size){

			printf("Error: Se deben lanzar %d procesos para un hipercubo de dimension %d \n", numeroProcesosALanzar, L);

			//Envio a los demas procesos que paren la ejecucion
			seguir=0;
			MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);

		}else{

			//Vector de numeros que repartiremos entre los nodos
			double *numeros=malloc(MAX_ITEMS * sizeof(double));

			//Leemos del fichero
			int cantidadNumeros=leerFichero(numeros);

			//Si la cantidad de numeros del fichero no es correcta
			if(pow(2,L)!=cantidadNumeros){
				printf("Error: Cantidad de numeros contenidos en el fichero incorrecto. Se necesitan %d datos, y se tienen %d.\n", numeroProcesosALanzar, cantidadNumeros);

				//Envio a los demas procesos que paren la ejecucion
				seguir=0;
				MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);

			}else{

				//Continuamos la ejecucion en los demas procesos
				MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);

				int j;

				//Envio los numeros del fichero, uno a cada rank
				for (j = 0; j < cantidadNumeros; ++j){
					bufferNumero=numeros[j];
					MPI_Send(&bufferNumero, 1, MPI_DOUBLE, j, 0, MPI_COMM_WORLD);
				}

				//Libero el espacio de memoria asignado para los numeros
				free(numeros);
			}


		}

	}

	//Espero un broadcast del root para saber si continuar
	MPI_Bcast(&seguir, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//Si el root nos permite seguir
	if(seguir!=0){

		//Recibo el dato correspondiente a mi rank
		MPI_Recv(&bufferNumero, 1, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		obtenerVecinos(rank, vecinos);
		numeroMaximo=obtenerMaximo(rank, bufferNumero, vecinos);

		//Solo el rank 0 imprime el menor de la red
		if(rank==0){
			printf("Soy el rank %d. El mayor numero de la red es: %.3lf\n", rank, numeroMaximo);
		}

	}

	//Finalizamos la ejecucion
	MPI_Finalize();
	return 0;
}

//Obtenemos los numeros del fichero para guardarlos en un array
int leerFichero(double *numeros){

	//Vector auxiliar de char para trabajar con los numeros del fichero
	char *listaNumeros=malloc(MAX_FILE * sizeof(char));

	//Tamanio del vector de numeros
	int cantidadNumeros=0;

	//Caracter auxiliar para trabajar con los numeros del fichero
	char *numeroActual;

	//Abrimos el fichero con permisos de lectura
	FILE *fichero=fopen(DATOSDAT, "r");
	if(!fichero){
		fprintf(stderr,"ERROR: no se pudo abrir el fichero\n.");
		return 0;
	}

	//Copiamos los datos del fichero al vector auxiliar de char
	fscanf(fichero,"%s",listaNumeros);

	//Cerramos el fichero
	fclose(fichero);

	//Leemos el primer numero hasta la primera coma. Usamos la funcion strtok. Con atof transformamos el string a double
	numeros[cantidadNumeros++]=atof(strtok(listaNumeros,","));

	//Vamos leyendo hasta que no haya mas numeros delante de las comas
	while( (numeroActual = strtok(NULL, "," )) != NULL ){
		//Metemos en el vector el numero correspondiente
		numeros[cantidadNumeros++]=atof(numeroActual);
	}

	return cantidadNumeros;
}

//Funcion que nos permitira obtener los rank de los vecinos correspondientes a mi rank
void obtenerVecinos(int rank, int *vecinosAdyacentes){
	int i;
	int rankAuxiliar;

	//Cada nodo tiene tantos vecinos como dimensiones
	for(i=0; i<L; i++){
		
		//Realizamos una XOR binaria entre el rank del nodo y el valor de sus adyacentes
		rankAuxiliar=(rank^((int)pow(2,i)));
		
		vecinosAdyacentes[i]=rankAuxiliar;
	}

}

//Funcion que nos permite obtener el mayor numero de toda la red
double obtenerMaximo(int rank, double bufferNumero, int *vecinos){

	int i;

	//Maximo numero de toda la red
	double maximo=DBL_MIN;

	MPI_Status status;

	//Para todos los vecinos de mi rank
	for(i=0;i<L;i++){

		//Si el numero actual es mayor que el maximo
		if(bufferNumero>maximo){
			maximo=bufferNumero;
		}


		//Envio mi maximo a mi vecino
		MPI_Send(&maximo, 1, MPI_DOUBLE, vecinos[i], i, MPI_COMM_WORLD);

		//Recibo de mi vecino el maximo
		MPI_Recv(&bufferNumero, 1, MPI_DOUBLE, vecinos[i], i, MPI_COMM_WORLD, &status);

		//Comparo si es mayor el maximo de mi vecino
		if(bufferNumero>maximo){
			maximo=bufferNumero;
		}
	}

	return maximo;

}