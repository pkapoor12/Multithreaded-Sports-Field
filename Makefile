all: stadium

stadium: stadium.c
	gcc -o stadium stadium.c

clean:
	rm -f stadium