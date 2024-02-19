import socket
import threading

class TCPServer:
    def __init__(self, host='192.168.0.102', port=60691, outputFile='received_data.txt'):
        self.host = host
        self.port = port
        self.outputFile = outputFile
        self.serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  # Allow address reuse
        self.clientSockets = []
        self.running = True

    def handle_client_connection(self, clientSocket, address):
        print(f"Accepted connection from {address}")
        with open(self.outputFile, 'a') as file:
            while True:
                try:
                    data = clientSocket.recv(1024)
                    if not data:
                        break
                    print(f"Received data: {data.decode().rstrip()}")
                    file.write(data.decode())
                    file.flush()
                except ConnectionResetError:
                    break
        print(f"Connection closed by {address}")
        clientSocket.close()
        self.clientSockets.remove(clientSocket)

    def start(self):
        self.serverSocket.bind((self.host, self.port))
        self.serverSocket.listen(5)
        print(f"Server listening on {self.host}:{self.port}")
        try:
            while self.running:
                clientSocket, address = self.serverSocket.accept()
                self.clientSockets.append(clientSocket)
                clientThread = threading.Thread(target=self.handle_client_connection, args=(clientSocket, address))
                clientThread.start()
        except KeyboardInterrupt:
            print("\nServer is shutting down.")
            self.running = False
        finally:
            for cs in self.clientSockets:
                cs.close()
            self.serverSocket.close()

if __name__ == '__main__':
    server = TCPServer(port=60691, outputFile='received_data.txt')
    try:
        server.start()
    except KeyboardInterrupt:
        print("\nServer is shutting down.")
        self.running = False
