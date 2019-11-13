import socket
import select
import sys

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = "127.0.0.1"
Port = 1025
server.connect((host, Port))

while True:
    sockets_list = [sys.stdin, server]
    read_sockets, write_socket, error_socket = select.select(
        sockets_list, [], [])

    for socks in read_sockets:
        if socks == server:
            message = socks.recv(2048)
            print(message.decode("utf-8"))
        else:
            message = sys.stdin.readline()
            server.send(message.encode('utf-8'))
            sys.stdout.write("<You>")
            sys.stdout.write(message)
            sys.stdout.flush()
server.close()
