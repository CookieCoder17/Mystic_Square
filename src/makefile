slidingpuzzle-v3: slidingpuzzle-v3.o sp-pipe-client.o sp-pipe-server.o
	gcc -o slidingpuzzle-v3 slidingpuzzle-v3.o sp-pipe-client.o sp-pipe-server.o

slidingpuzzle-v3.o: slidingpuzzle-v3.c 
	gcc -Wall -c slidingpuzzle-v3.c

sp-pipe-client.o: sp-pipe-client.c
	gcc -Wall -c sp-pipe-client.c

sp-pipe-server.o: sp-pipe-server.c 
	gcc -Wall -c sp-pipe-server.c

clean: 
	rm *.o slidingpuzzle-v3
