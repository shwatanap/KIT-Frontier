ODIR		= objs
SDIR		= srcs
BDIR		= bin
DDIR		= dots
SRCS		= frontier.cpp
OBJS		= $(addprefix $(ODIR)/, $(SRCS:.cpp=.o))
NAME		= $(BDIR)/path
HSRCS		= hamilton.cpp
HOBJS		= $(addprefix $(ODIR)/, $(HSRCS:.cpp=.o))
HNAME		= $(BDIR)/hamilton
INCDIR		= TdZdd/include
NUM			= 2
CPP			= g++
RM			= rm -rf

exec: $(NAME)
	./$(NAME) grid/grid$(NUM)x$(NUM).grh

$(NAME): $(BDIR) $(ODIR) $(OBJS)
	$(CPP) -o $(NAME) $(OBJS)

exech: $(HNAME)
	./$(HNAME) grid/grid$(NUM)x$(NUM).grh

$(HNAME): $(BDIR) $(ODIR) $(HOBJS)
	$(CPP) -o $(HNAME) $(HOBJS)

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CPP) -I $(INCDIR) -c $< -o $@

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

re: fclean exec

dot: $(DDIR) $(NAME)
	./$(NAME) grid/grid$(NUM)x$(NUM).grh > $(DDIR)/output.dot
	dot -Tpng $(DDIR)/output.dot -o $(DDIR)/output.png

doth: $(DDIR) $(HNAME)
	./$(HNAME) grid/grid$(NUM)x$(NUM).grh > $(DDIR)/output.dot
	dot -Tpng $(DDIR)/output.dot -o $(DDIR)/output.png

.PHONY: clean fclean dot
