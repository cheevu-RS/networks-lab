import socket

host = '127.0.0.1'
port = 6961
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))
while True:
    data = raw_input("ENter text\n")
    s.sendall(data.encode('ascii'))
    data = s.recv(1024)
    if not data:
        break
    print "Server replied ", data.decode('ascii')
