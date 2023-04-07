import socket
import threading
import time
import struct

class TCPServer:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.client_socket = None
        self.is_running = False

    def start(self):
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen()
        print(f"Listening on {self.host}:{self.port}")
        self.is_running = True
        # Thread for accepting connections
        conn_thread = threading.Thread(target=self._accept_connections, daemon=True)
        conn_thread.start()

        # Thread for receiving input from client
        recv_thread = threading.Thread(target=self._recv_data, daemon=True)
        recv_thread.start()

        # Thread for sending input from keyboard
        send_thread = threading.Thread(target=self._send_data, daemon=True)
        send_thread.start()

    def stop(self):
        self.is_running = False
        if self.client_socket:
            self.client_socket.close()
        self.server_socket.close()
        print("Server stopped.")

    def _accept_connections(self):
        while self.is_running:
            if not self.client_socket:
                self.client_socket, client_address = self.server_socket.accept()
                print(f"Connected by {client_address}")
            else:
                pass
                time.sleep(1)  

    def _recv_data(self):
        while self.is_running:
            if self.client_socket:
                try:
                    data = self.client_socket.recv(1024)
                    if not data:
                        break
                    numbers2 = struct.unpack("<" + "H" * (len(data) // 2), data)
                    numbers = struct.unpack(">" + "H" * (len(data) // 2), data)
                    #print(data.decode(), end='')
                    print("< numbers2 \r\n")
                    print(numbers2)
                    print(" numbers >\r\n")
                    print(numbers)

                except ConnectionResetError:
                    print("\nConnection with client reset.")
                    self.client_socket = None
            time.sleep(0.001)

    def _send_data(self):
        while self.is_running:
            if self.client_socket:
                try:
                    message = input()
                    if message:
                        message += '\r\n'
                        self.client_socket.sendall(message.encode())
                except ConnectionResetError:
                    print("\nConnection with client reset.")
                    self.client_socket = None
            time.sleep(0.01)

if __name__ == "__main__":
    HOST = "192.168.0.105"  # The server's hostname or IP address
    PORT = 65432  # The port used by the server
    server = TCPServer(HOST, PORT)
    server.start()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nKeyboard interrupt received. Stopping server.")
        server.stop()

