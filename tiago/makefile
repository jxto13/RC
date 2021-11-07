
driver: stateM_lib.o main.c
	@gcc -Wall -o main main.c stateM_lib.o
	
stateM_lib.o: stateM_lib.h stateM_lib.c
	@gcc -Wall -Wextra -fPIC stateM_lib.h -c stateM_lib.c


reciver: stateM_lib.o noncanonicalH.c
	@gcc -Wall -o reader noncanonicalH.c stateM_lib.o

run: 
	@./main

run_reader:
	@./reader /dev/ttyS11
	

clean:
	@rm -rf *.o *.gch main