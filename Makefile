.PHONY : clean 

server : main.o http.o
	gcc main.o http.o  -lpthread -o server

main.o : main.c
	gcc -c main.c -o main.o 

http.o : http.c
	gcc -c http.c -o http.o

clean : 
	rm -f ./*.o 
