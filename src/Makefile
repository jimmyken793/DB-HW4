#
# Makefile for CS564 Minibase project.  Needs GNU make.
#
# Define DEBUGREL for some kind of debugging output (not from us, from
# the original Minibase implementors.)
#
# Warning: make depend overwrites this file.

.PHONY: depend clean backup setup

MAIN=buftest

MINIBASE= ..

CC=g++

CFLAGS= -DUNIX -Wall -g

INCLUDES = -I${MINIBASE}/include -I.

LFLAGS= -L${MINIBASE}/lib  -liberty -lwithout -lm -g 

SRCS = main.C buf.C BMTester.C test_driver.C db.C new_error.C page.C system_defs.C #bufHash.C

OBJS = $(SRCS:.C=.o)

$(MAIN):  $(OBJS)
	 $(CC) $(CFLAGS) $(INCLUDES) $(OBJS)  -o $(MAIN) #$(LFLAGS)

.C.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

depend: $(SRCS)
	makedepend $(INCLUDES) $^

clean:
	rm -f *.o *~ $(MAIN)
	rm -f my_output

run:
	rm -f my_output
	./$(MAIN) > my_output

backup:
	-mkdir bak
	cp Makefile *.[Ch] bak

# Grab the sources for a user who has only the makefile
setup:
	/bin/cp -i $(MINIBASE)/src/*.[Ch] .
	/bin/cp -i $(MINIBASE)/src/*.sample .

# DO NOT DELETE THIS LINE -- make depend needs it 
