all: utils.o pci.o mmio.o msr.o pmu.o imc.o cha.o IMC-raw

IMC-raw: IMC-raw.cpp utils.o pci.o mmio.o msr.o pmu.o imc.o cha.o
	g++ IMC-raw.cpp utils.o pci.o mmio.o msr.o pmu.o imc.o cha.o -o IMC-raw.x 

utils.o: utils.h utils.cpp global.h types.h
	g++ -c utils.cpp -o utils.o

pci.o: pci.h pci.cpp global.h types.h
	g++ -c pci.cpp -o pci.o

msr.o: msr.h msr.cpp global.h types.h
	g++ -c msr.cpp -o msr.o

pmu.o: pmu.h pmu.cpp global.h types.h
	g++ -c pmu.cpp -o pmu.o

imc.o: imc.h imc.cpp global.h types.h
	g++ -c imc.cpp -o imc.o

cha.o: cha.h cha.cpp global.h types.h
	g++ -c cha.cpp -o cha.o

mmio.o: mmio.h mmio.cpp global.h types.h
	g++ -c mmio.cpp -o mmio.o

debug: $(wildcard *.h) $(wildcard *.cpp)
	g++ -g -o debug.x $(wildcard *.cpp)

clean:
	rm -rf *.x *.o *~ *.d *.a *.so

remake: clean all

run:
	./IMC-raw.x -e imc/config=0x000000000000f005,name=UNC_M_CAS_COUNT.WR -e imc/config=0x000000000000cf05,name=UNC_M_CAS_COUNT.RD  -e imc/config=0x0000000000000082,name=UNC_M_WPQ_OCCUPANCY_PCH0 -e imc/config=0x0000000000000080,name=UNC_M_RPQ_OCCUPANCY_PCH0 -d 1 -e cha/config=0x00c001fe00010135,config1=0x20,name=UNC_CHA_TOR_INSERTS.IA_MISS:tid=0x20

cha:
	./IMC-raw.x -e cha/config=0x00c001fe00010135,config1=0x20,name=UNC_CHA_TOR_INSERTS.IA_MISS:tid=0x20