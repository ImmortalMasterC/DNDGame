CFLAGS = -O -DCOMPRESS
LIBS = -lm 
CC = cc

OBJECTS = editor.o editor2.o editor3.o editor4.o compress.o

editor: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o editor $(LIBS)
	chmod o-rwx editor 

clean: 
	rm $(OBJECTS) $(OBJECTS2)

