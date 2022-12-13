all: filter_library.so

filter_library.so: filter_library.c
	gcc -O2 -shared -fPIC $^ -o $@

clean:
	rm -f filter_library.so *.pyc