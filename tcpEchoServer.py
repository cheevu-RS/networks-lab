import socket

host = '127.0.0.1'
port = 6961
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((host, port))
s.listen(10)
while True:
    conn, addr = s.accept()
    while True:
        data = conn.recv(1024)
        print "Client sent ", data
        if not data:
            break
        conn.sendall(data[::-1])
    conn.close()
