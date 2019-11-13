import socket

host = '127.0.0.1'
port = 6969
server_address = (host, port)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
while True:
    data = raw_input("ENter text\n")
    mess = data.encode('ascii')
    sent = s.sendto(mess, server_address)
    data, server = s.recvfrom(1024)
    if not data:
        break
    print("Server replied ", data.decode('ascii'))
