prog:	slidingpuzzle-v3.o sp-pipe-server.o sp-pipe-client.o messages.h
	gcc slidingpuzzle-v3.o sp-pipe-server.o sp-pipe-client.o puzzle -o prog
slidingpuzzle-v3.o: slidingpuzzle-v3.c
	gcc -c slidingpuzzle-v3.c
sp-pipe-server.o: sp-pipe-server.c
	gcc -c sp-pipe-server.c
sp-pipe-client.o: sp-pipe-client.c
	gcc -c sp-pipe-client.c
clean:
	rm sp-pipe-client.o sp-pipe-server.o slidingpuzzle-v3.o puzzle
