
#CC= /home/a860/crosstool/gcc3.4.1/usr/local/arm/3.4.1/bin/arm-linux-gcc
#AR= /home/a860/crosstool/gcc3.4.1/usr/local/arm/3.4.1/bin/arm-linux-ar
#EXEC = radio.a
#OBJS = *.c
#OBJ = radio.o
NAME = client_tcp
#SQLITELIB = libsqlite3.so.0
#LDFLAGS = $(SQLITELIB)
#CFLAGS = -g -rdynamic
#CFLAGS += -g
#all:$(NAME)

#$(NAME):
#$(CC) *.c $(CFLAGS) -o $(NAME) -Wall -lpthread $(LDFLAGS) 
all:
	$(CC) *.c $(CFLAGS) -o $(NAME) -lpthread $(LDFLAGS) 

clean:
	-rm -f $(NAME) 
