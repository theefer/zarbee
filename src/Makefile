CC	=	g++
CFLAGS	=	-ggdb3 -Wall -Woverloaded-virtual $(shell xml2-config --cflags) $(shell sdl-config --cflags)
LDFLAGS =	-lGL -lGLU -lglut -lxml2 -lpng $(shell sdl-config --libs) -lSDL -lSDL_mixer
DPFLAGS =	-MM

BASESRC =	main.cc world.cc zar_nodes.cc zar_parser.cc zar_param_table.cc bee_base.cc bee_primitives.cc bee_modifiers.cc camera.cc light.cc exception.cc time.cc texture.cc music.cc
SOURCES =	$(BASESRC)

BASEHDR =	world.hh zar_nodes.hh zar_parser.hh zar_param_table.hh bee_base.hh bee_primitives.hh bee_modifiers.hh camera.hh light.hh exception.hh time.hh texture.hh music.hh
HEADERS =	$(BASEHDR)

OBJECTS =	$(SOURCES:%.cc=%.o)
OUTFILE =	zarbee

DPFILE  =	Makefile.dependencies

all : $(OUTFILE) zar_nodes_test zar_parser_test

$(OUTFILE) : $(OBJECTS)
	$(CC) -o $(OUTFILE) $(OBJECTS) $(LDFLAGS) -s 

zar_nodes_test : zar_nodes_test.o zar_nodes.o exception.o

zar_parser_test : zar_parser_test.o zar_parser.o zar_param_table.o zar_nodes.o bee_base.o bee_primitives.o exception.o camera.o light.o world.o bee_modifiers.o time.o texture.o music.o

.cc.o: $(DPFILE)
	$(CC) $(CFLAGS) -c $<

clean : 
	rm -f $(OBJECTS) $(OUTFILE) zar_nodes_test zar_parser_test core *~ $(DPFILE)
	touch $(DPFILE)


$(DPFILE) depend : $(BASESRC) $(HEADERS) $(SOURCES)
	$(CC) $(DPFLAGS) $(CFLAGS) $(BASESRC) > $(DPFILE)

include $(DPFILE)
