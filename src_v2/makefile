
driver:
	gcc -Wall app.c link.c stateM_lib.c byteStuffing.c -o main -lm

run_emissor:
	@./main /dev/ttyS10 

run_reader:
	@./main /dev/ttyS11

openVport:
	sudo socat -d -d PTY,link=/dev/ttyS10,mode=777 PTY,link=/dev/ttyS11,mode=777

clean:
	@rm -rf *.o *.gch main
