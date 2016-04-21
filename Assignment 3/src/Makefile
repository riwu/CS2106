CC=g++
CFLAGS=-I .
DEPS = efs.h libefs.h

MAKEFSOBJ = makefs.o efs.o
TESTWOBJ = testwrite.o efs.o
TESTROBJ = testread.o efs.o
CHECKINOBJ = checkin.o efs.o libefs.o
CHECKOUTOBJ = checkout.o efs.o libefs.o
DELFILEOBJ = delfile.o efs.o libefs.o
FILEATTROBJ = attrfile.o efs.o libefs.o
GETATTROBJ = getattr.o efs.o libefs.o

ALL=makefs testwrite testread checkin checkout delfile attrfile getattr
all: $(ALL)

clean: 
	rm -f *.o
	rm -f $(ALL)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

makefs: $(MAKEFSOBJ)
	$(CC) -o $@ $^ $(CFLAGS)

testwrite: $(TESTWOBJ)
	$(CC) -o $@ $^ $(CFLAGS)

testread: $(TESTROBJ)
	$(CC) -o $@ $^ $(CFLAGS)

checkin: $(CHECKINOBJ)
	$(CC) -o $@ $^ $(CFLAGS)

checkout: $(CHECKOUTOBJ)
	$(CC) -o $@ $^ $(CFLAGS)

delfile: $(DELFILEOBJ)
	$(CC) -o $@ $^ $(CFLAGS)

attrfile: $(FILEATTROBJ)
	$(CC) -o $@ $^ $(CFLAGS)

getattr: $(GETATTROBJ)
	$(CC) -o $@ $^ $(CFLAGS)
