# Nom de l'executable
NAME = emu

CC = gcc # Déclaration d'un variable pour y l'utiliser $(CC)
CFLAGS	= -Wall -Wextra -Werror

SRCDIR = src
INCDIR = includes
LIBDIR = lib
BINDIR = bin
OBJSDIR = obj
SRCS = main.c  \
	   cpu.c   \
	   cart.c  \
	   bus.c   \
	   ram.c   \
	   io.c    \
	   timer.c \
	   ppu.c   \
	   video.c \
	   lcd.c   \
	   joypad.c \
	   oam_dma.c \

LIBFLAG = -L $(LIBDIR) -l SDL3 # Permet de compiler SDL

OBJS = $(addprefix $(OBJSDIR)/, $(SRCS:.c=.o)) # addprefix: ajoute le prefixe OBJSDIR/ devant toutes les valuers | $(SRCS:.c=.o): Change toutes les extensions en .o

# Cible: dependance
all: $(NAME)

$(NAME): $(OBJS) | $(BINDIR) # Avec le |, $(OBJSDIR) est une dépendance d’ordre : Make s’assure juste que le dossier existe, mais sa modification ne force pas la recompilation des .o
	$(CC) -o $(BINDIR)/$@ $^ $(LIBFLAG)

$(OBJSDIR)/%.o: $(SRCDIR)/%.c | $(OBJSDIR)# Toutes les cibles en .o je vais les créer à partir de toutes les dépendances .c
	$(CC) $(CFLAGS) -I $(INCDIR) -c $< -o $@ 
# $< va print la premiere dependance ici vu qu'il y a toujours une dépendance ca sera toujours %c

$(OBJSDIR):
	mkdir $(OBJSDIR)

$(BINDIR):
	mkdir $(BINDIR)

clean:
	del /s /q *.o

fclean: clean
	del /s /q $(BINDIR)\$(NAME).exe

re: fclean $(NAME)

.PHONY:	all clean fclean re
# Variables spéciale
# $@ Nom de la cible
# $< Nom première dépendance
# $^ Liste dépendances
