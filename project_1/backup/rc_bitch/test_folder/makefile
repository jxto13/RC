
emissor: writenoncanonical.c
	@gcc -Wall -o emissor writenoncanonical.c 

reciver: noncanonical.c
	@gcc -Wall -o reader noncanonical.c

run_reader:
	@./reader /dev/ttyS11
run_emissor:
	@./emissor /dev/ttyS10

clean:
	@rm -rf *.o *.gch main
