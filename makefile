NAME = webserv
CC = clang++
CFLAGS = -Wall -Wextra -Werror 
SRC = srcs/webServer.cpp\
	  srcs/HttpServer/*.cpp\
	  srcs/FileParser/*.cpp\
	  srcs/Request/*.cpp\
	  srcs/Request/Cstring/*.cpp\
	  srcs/Request/utils/*.cpp\
	  srcs/Response/*.cpp\
	  srcs/Response/utils/*.cpp\
	  srcs/Checker/checker.cpp\
	  srcs/Checker/utils/*.cpp\
	  srcs/Cgi/*.cpp
all : $(NAME)
$(NAME): $(SRC)
	@$(CC) $(CFLAGS) $(SRC) -o $(NAME)
clean:
	@rm -rf *.o

fclean: clean
	@rm -rf $(NAME) 
re:	fclean all