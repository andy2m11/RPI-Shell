$(shell export LD_LIBRARY_PATH=$PWD:$LD_LIBRARY_PATH)

#Time: start 3:00
PROGS = tshell exshell tshell2 finds
#OBJS = objserver.o
#LIBS = libobjserver.a #libobjserver.so
all: $(LIBS) $(PROGS) $(OBJS)


exshell: shell_example.c
	gcc -o $@ shell_example.c

tshell: tshell.c
	gcc -o $@ tshell.c

tshell2: tshell2.c
	gcc -o $@ tshell2.c

	
finds: getopt.c
	gcc -o $@ getopt.c
	
	
clean:
	rm -f *.0 *~ *.so *.a $(PROGS)


