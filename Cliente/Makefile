# PROGRAMA
    PROG = game
    OBJS = game.o mapa_cliente.o raw_sockets.o libArquivo.o enviar.o receber.o # acrescentar aqui outros módulos do programa

# Compilador
    CC     = gcc
    CFLAGS = -Wall 

.PHONY: all clean purge

%.o: %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

all: $(PROG)

$(PROG): % : %.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

	make clean

clean:
	@rm -f *~ *.o

purge:  clean
	@rm -f $(PROG) 
