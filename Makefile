LADO := 3
 
all: desplegarHipercubo

limpiarDirectorio:
	-rm -f hipercubo
	-rm -f datos.dat
	-rm -f generarDatos
	
generarDatos:
	gcc generar_datos.c -o generarDatos
	@read -p "Introduce el numero de datos a generar en el fichero datos.dat: " numeroDatos; \
	./generarDatos $$numeroDatos

compilarHipercubo: 
	@read -p "Introduce la dimension del hipercubo: " dimension; \
	mpicc -g hipercubo.c -lm -o hipercubo -DL=$$dimension

ejecutarHipercubo: 
	@read -p "Numero de procesos para lanzar el hipercubo: " procesos; \
	mpirun -n $$procesos hipercubo

desplegarHipercubo: limpiarDirectorio generarDatos compilarHipercubo ejecutarHipercubo
