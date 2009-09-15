CC		= gcc
CFLAGS		= -O0 -Wall -pipe -g 
#CFLAGS = -O3 -pipe -fomit-frame-pointer
#-D_DATA_PATH=\"${DAT_DIR}\" 
LDFLAGS		= -lSDL -lGL -lGLU -lSDL_image -lm
#LDFLAGS	= -lSDL -lEGL -lGLESv2 -lm -lSDL_image
NAME		= dman
OBJECTS		= main.o \
		graphics.o \
		config.o \
		mainloop.o \
		message.o \
		input.o \
		sprites.o \
		splash.o \
		menu.o \
		snge.o \
		exitscr.o \
		common.o \
		glout.o \
		game.o \
		generator.o \
                level.o \
                player.o \
                timer.o \
                stack.o \
                bcg.o \
                particles.o \
                glesout.o
			
SOURCES		= $(OBJECTS:.o=.c)

PREFIX		= /usr
LIB_DIR		= ${PREFIX}/lib
BIN_DIR		= ${PREFIX}/bin
DAT_DIR		= ${PREFIX}/share/${NAME}/

INSTALL = install -c
INSTALL_PROGRAM = ${INSTALL} 
INSTALL_DATA = ${INSTALL} -m 644

all: dep $(NAME) 

rebuild: clean all

purge: clean
	$(RM) *~

dep: .depend

.depend:
	$(CC) -MM $(CFLAGS) $(SOURCES) 1> .depend

$(NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	
install:
	
	$(INSTALL_PROGRAM) $(NAME) $(BIN_DIR)
	$(INSTALL_PROGRAM) -d $(DAT_DIR)
	cp -r data/* $(DAT_DIR)
	
	
	
uninstall:
	$(RM) $(BIN_DIR)/$(NAME)

clean:
	$(RM) $(NAME) *.o .depend

.PHONY: clean purge rebuild

ifneq ($(wildcard .depend),)
include .depend
endif
