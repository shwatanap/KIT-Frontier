SRCS		= frontier2.cpp
OBJS		= $(SRCS:.cpp=.o)
NAME		= path
INCDIR		= TdZdd/include
NUM			= 2
RM			= rm -rf

exec: $(NAME)
	./$(NAME) grid/grid$(NUM)x$(NUM).grh

$(NAME): $(OBJS)
	g++ -o $(NAME) $^

%.o: %.cpp
	g++ -I $(INCDIR) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

oclean:
	$(RM) *.o

re: fclean exec

dot: $(NAME)
	./$(NAME) grid/grid$(NUM)x$(NUM).grh > output.dot
	dot -Tpng output.dot -o output.png

.PHONY: clean fclean
