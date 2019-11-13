import socket

host = '127.0.0.1'
port = 6969

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((host, port))
data, addr = s.recvfrom(1024)
if data:
    print("Received ", data, " from ", addr)
