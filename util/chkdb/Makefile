CFLAGS = -g -m486 -O2
LIBS =  
CC = cc

OBJECTS = utils.o ckdb.o


chkdb: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o chkdb $(LIBS)
clean: 
	\rm $(OBJECTS) core

