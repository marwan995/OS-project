build:
	gcc process_generator.c -o process_generator.out -lm
	gcc clk.c -o clk.out -lm
	gcc scheduler.c -o scheduler.out -lm
	gcc process.c -o process.out -lm
	gcc test_generator.c -o test_generator.out -lm

clean:
	rm -f *.out  processes.txt
	
test:
	./test_generator.out

all: clean build test run

run:
	./process_generator.out || exit 1
