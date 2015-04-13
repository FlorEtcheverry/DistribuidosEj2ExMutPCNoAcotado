all: project

clean:
	rm -rf ./build
	rm -rf ./dist
	rm -rf /home/knoppix/AlvarezEtcheverry/Ejercicio2

project:
	mkdir -p /home/knoppix/AlvarezEtcheverry/Ejercicio2/v1
	mkdir -p /home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/museo
	mkdir -p /home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/museolleno
	mkdir -p /home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/colaentrada
	mkdir -p /home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/colaentradarespuesta
	mkdir -p /home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/colasalida
	mkdir -p /home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/colasalidarespuesta
	mkdir -p /home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/colamuseocerrado
	cp ./init.conf /home/knoppix/AlvarezEtcheverry/Ejercicio2/v1
	mkdir -p ./build
	mkdir -p ./dist
	g++ -Wall Logger.cpp -c -o ./build/Logger.o
	g++ -Wall Semaforo.cpp -c -o ./build/Semaforo.o
	g++ -Wall ./build/Logger.o ./build/Semaforo.o initializer.cpp -o ./dist/initializer
	g++ -Wall ./build/Logger.o ./build/Semaforo.o puertaEntrada.cpp -o ./dist/puertaEntrada
	g++ -Wall ./build/Logger.o ./build/Semaforo.o puertaSalida.cpp -o ./dist/puertaSalida
	g++ -Wall ./build/Logger.o ./build/Semaforo.o destroyer.cpp -o ./dist/destroyer
	g++ -Wall ./build/Logger.o ./build/Semaforo.o abrirMuseo.cpp -o ./dist/abrirMuseo
	g++ -Wall ./build/Logger.o ./build/Semaforo.o cerrarMuseo.cpp -o ./dist/cerrarMuseo