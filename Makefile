ODIR		= objs
SDIR		= srcs
BDIR		= bin
DDIR		= dots
SRCS		= frontier.cpp
OBJS		= $(addprefix $(ODIR)/, $(SRCS:.cpp=.o))
NAME		= $(BDIR)/path
HSRCS		= hamilton.cpp
HOBJS		= $(addprefix $(ODIR)/, $(SRCS:.cpp=.o))
HNAME		= $(BDIR)/hamilton
INCDIR		= TdZdd/include
NUM			= 2
RM			= rm -rf

exec: $(NAME)
	./$(NAME) grid/grid$(NUM)x$(NUM).grh

$(NAME): $(BDIR) $(ODIR) $(OBJS)
	g++ -o $(NAME) $(OBJS)

exech: $(ODIR) $(HNAME)
	./$(HNAME) grid/grid$(NUM)x$(NUM).grh

$(HNAME): $(BDIR) $(ODIR) $(HOBJS)
	g++ -o $(HNAME) $^

$(ODIR)/%.o: $(SDIR)/%.cpp
	g++ -I $(INCDIR) -c $< -o $@

$(BDIR):
	mkdir -p $@

$(ODIR):
	mkdir -p $@

$(DDIR):
	mkdir -p $@

clean:
	$(RM) $(ODIR)

fclean: clean
	$(RM) $(BDIR)

oclean:
	$(RM) *.o

re: fclean exec

dot: $(DDIR) $(NAME)
	./$(NAME) grid/grid$(NUM)x$(NUM).grh > $(DDIR)/output.dot
	dot -Tpng $(DDIR)/output.dot -o $(DDIR)/output.png

.PHONY: clean fclean dot
