objects = main.o parser.o system.o subsetsum.o

sched : $(objects)
	g++ -o cadb032 $(objects)

main.o : parser.h system.h

parser.o: parser.h system.h

system.o: system.h subsetsum.h

subsetsum.o:  subsetsum.h

clean : 
	rm cadb032 $(objects)
