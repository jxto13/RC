# RC

comando de compilação
gcc -Wall write.c -o write && ./write /dev/ttyS0

terminal 1:
sudo socat -d  -d  PTY,link=/dev/ttyS10,mode=777   PTY,link=/dev/ttyS10,mode=777

terminal 2:
sudo socat -d  -d  PTY,link=/dev/ttyS11,mode=777   PTY,link=/dev/ttyS10,mode=777

correr o programa de enviar:
make emissor run_emissor

correr o programa de receber: 
make reciver run_reader
