.PHONY: clean write control

all: write control

write:
	gcc -o write write.c

control:
	gcc -o control control.c

run_write:
	./write

run_control:
	./control

clean:
	rm write
	rm control
