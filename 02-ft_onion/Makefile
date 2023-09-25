NAME = ft_onion
PORT_LINK_1 = 80:80
PORT_LINK_2 = 4242:4242

all:		build run

build:
		docker build -t $(NAME) .

run:
		docker run --name $(NAME) -d -p $(PORT_LINK_1) -p $(PORT_LINK_2) $(NAME)

enter:
		docker exec -it $(NAME) /bin/bash

stop:
		docker stop $(NAME)

start:
		docker start $(NAME)

restart:
		docker restart $(NAME)

delete-ctn:
		docker container rm $(NAME)

delete-img:
		docker rmi $(NAME)

clean:
		docker system prune -f

fclean:
		docker system prune -fa

.PHONY:		all build run enter stop start restart	\
		delete-ctn delete-img clean fclean