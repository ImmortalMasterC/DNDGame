
# Read the NOTES file to see more information on the various 
# CFLAG options.

CFLAGS = -DCOMPRESS -DFINGERACCT -DSUICIDE $(OSFLAGS)
# Include these CFLAGS for more verbose warnings
# -W -Wall -pedantic

LIBS = -lm -lc
CC = cc -g
OUTFILE = DAR

OBJECTS = global.o files1.o files2.o files3.o files4.o command1.o \
          command2.o command3.o command4.o command5.o command6.o command7.o \
          command8.o command9.o command10.o command11.o command12.o command13.o \
          command14.o command15.o magic1.o magic2.o magic3.o magic4.o magic5.o \
          magic6.o magic7.o magic8.o magic9.o screen.o  talk.o \
          bank.o dm1.o dm2.o dm3.o dm4.o dm5.o dm6.o dm7.o dm8.o edit.o \
          player.o creature.o room.o object.o update.o post.o action.o io.o \
          misc.o whispers.o special1.o compress.o memory.o combat.o logic.o


OBJECTS2 = files1.o global.o

$(OUTFILE): $(OBJECTS) main.o
	$(CC) $(CFLAGS) $(OFLAGS) $(OBJECTS) main.o -o $(OUTFILE) $(LIBS)
	chmod 700 $(OUTFILE)
#	strip $(OUTFILE)

auth: auth.o
	$(CC) auth.o -o auth
	mv auth ../bin

clean: 
	rm main.o $(OBJECTS)

port:
	rm -f io.o main.o;
	make

fing: finger.o
	$(CC) finger.o -o fing
	mv fing ../bin

plist: plist.o $(OBJECTS) 
	$(CC) $(CFLAGS) $(OFLAGS) $(OBJECTS) plist.o -o plist $(LIBS)
	mv plist ../bin

gstat: gstat.o $(OBJECTS) 
	$(CC) $(CFLAGS) $(OFLAGS) $(OBJECTS) gstat.o -o gstat $(LIBS)
	mv gstat ../bin

psearch: psearch.o $(OBJECTS)
	$(CC) $(CFLAGS) $(OFLAGS) $(OBJECTS) psearch.o -o psearch $(LIBS)
	mv psearch ../bin
