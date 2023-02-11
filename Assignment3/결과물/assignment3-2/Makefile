
all:
	rm -rf tmp*
	sync
	echo 3| sudo tee /proc/sys/vm/drop_caches
	gcc -o filegen filegen.c
	gcc -o schedtest schedtest.c

clean:
	rm -f filegen
	rm -f schedtest
	rm -rf temp

