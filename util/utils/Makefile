CC = gcc
CFLAGS = -O2 -DCOMPRESS
GOBJECTS = gstat.o util.o compress.o
POBJECTS = plist.o util.o compress.o
PSOBJECTS = psearch.o util.o compress.o util2.o

gstat: $(GOBJECTS)
	$(CC) $(CFLAGS) -o gstat $(GOBJECTS)
	
plist: $(POBJECTS)
	$(CC) $(CFLAGS) -o plist $(POBJECTS)

tdel: 
	$(CC) $(CFLAGS) -o tdel tdel.c

psearch: $(PSOBJECTS)
	$(CC) $(CFLAGS) -o psearch $(PSOBJECTS)

all: gstat plist tdel psearch

clean:
	rm *.o gstat plist tdel psearch

