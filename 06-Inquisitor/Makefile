NAME		= Inquisitor
CC		= c++ -Wall -Werror -Wextra -g

INCLUDE_DIR	= include
HEADER_EXT	= hpp
HEADER		= $(shell find $(INCLUDE_DIR) -type f -name "*.$(HEADER_EXT)")

SRC_DIR		= src
SRC_EXT		= cpp
SRC_FILES	= $(wildcard $(SRC_DIR)/*/*.$(SRC_EXT))

OBJ_DIR		= obj
OBJ_SUBDIR	= $(shell find $(SRC_DIR) -type d | grep '/' | sed 's/$(SRC_DIR)/$(OBJ_DIR)/g')
OBJ_FILES	= $(patsubst $(SRC_DIR)/%.$(SRC_EXT), $(OBJ_DIR)/%.o, $(SRC_FILES))

LIB		= -lpcap

all		: $(NAME)

$(NAME)		: $(OBJ_DIR) $(OBJ_SUBDIR) $(OBJ_FILES)
		$(CC) $(OBJ_FILES) -o $@ $(LIB)
		@echo
		@echo "You can now 'make docker'."
		@echo

$(OBJ_DIR)	:
		@mkdir -p $(OBJ_DIR)

$(OBJ_SUBDIR)	:
		@mkdir -p $(OBJ_SUBDIR)

$(OBJ_DIR)/%.o	: $(SRC_DIR)/%.$(SRC_EXT) $(HEADER)
		$(CC) -c $< -o $(<:.$(SRC_EXT)=.o)
		@mv $(SRC_DIR)/*/*.o $@

clean		:
		rm -rf $(OBJ_DIR)

fclean		: clean
		rm -f $(NAME)

re		: fclean all

docker		:
		docker-compose build
		docker-compose up -d

docker-stop	:
		docker-compose down

docker-clean	:
		docker system prune -f

docker-fclean	:
		docker system prune -fa

docker-re	: docker-fclean docker

.PHONY		: all clean fclean re docker docker-stop docker-clean docker-fclean docker-re