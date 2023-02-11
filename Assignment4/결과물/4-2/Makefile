EXEC = D_recompile

CC = gcc

default:
	$(CC) 	-o drecompile D_recompile.c
dynamic:
	$(CC) -Ddynamic -o drecompile D_recompile.c
test2:
	$(CC) -o test2 D_recompile_test.c
objdump:
	$(CC) -c D_recompile_test.c
	objdump -d D_recompile_test.o > test
result_helper:
	gcc -o result_helper result_helper.c
clean: 
	rm -rf drecompile $(EXEC)
	rm -rf test2
	rm -rf D_recompile_test.o
	rm -rf test

