all: utils.o pci.o pmu.o imc.o IMC-raw

IMC-raw: IMC-raw.cpp IMC-raw.h $(wildcard *.o)
	g++ -o IMC-raw IMC-raw.cpp $(wildcard *.o)

utils.o: utils.h utils.cpp global.h types.h
	g++ -c utils.cpp -o utils.o

pci.o: pci.h pci.cpp global.h types.h
	g++ -c pci.cpp -o pci.o

pmu.o: pmu.h pmu.cpp global.h types.h
	g++ -c pmu.cpp -o pci.o

imc.o: imc.h imc.cpp global.h types.h
	g++ -c imc.cpp -o imc.o

clean:
	rm -rf *.x *.o *~ *.d *.a *.so
