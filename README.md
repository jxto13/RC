# RC

comando de compilação
gcc -Wall write.c -o write && ./write /dev/ttyS0

virtual cables:
sudo socat -d  -d  PTY,link=/dev/ttyS10,mode=777   PTY,link=/dev/ttyS11,mode=777

correr o programa de enviar:
make emissor run_emissor

correr o programa de receber: 
make reciver run_reader
