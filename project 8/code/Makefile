CC=gcc
CFLAGS=-Wall

vm_manager: vm_manager.o data_structure.o
	$(CC) $(CFLAGS) -o vm_manager vm_manager.o data_structure.o 

vm_manager.o: vm_manager.c
	$(CC) $(CFLAGS) -c vm_manager.c

data_structure.o: data_structure.c data_structure.h
	$(CC) $(CFLAG) -c data_structure.c

clean: 
	rm -rf *.o
	rm -rf vm_manager