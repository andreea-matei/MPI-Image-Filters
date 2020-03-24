build: tema.c
	mpicc -o tema3 tema.c -lm -Wall
clean:
	rm -f tema
	
