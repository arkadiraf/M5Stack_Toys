import socket
import threading
import time
import struct
import matplotlib.pyplot as plt
import numpy as np
import sys

class TCPServer:
    def __init__(self, host = "192.168.0.105", port = 65432, timeout=1):
        self.host = host
        self.port = port
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.settimeout(timeout)
        self.client_socket = None
        self.is_running = False
        self.fig, self.ax = plt.subplots()
        self.fig.canvas.mpl_connect('close_event', self._handle_close) # register a callback for the plot window closing
        self.numpoints = 500
        self.plot_data = np.zeros(self.numpoints)
        time.sleep(0.1) #allow some time for the plot to start
        

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
        time.sleep(1) #wait for the processes to finish
        if self.client_socket:
            self.client_socket.close()
        self.server_socket.close()
        print("Server stopped.")
        sys.exit()

    def _accept_connections(self):
        while self.is_running:
            try:
                if not self.client_socket:
                    self.client_socket, client_address = self.server_socket.accept()
                    self.client_socket.settimeout(30.0)
                    print(f"Connected by {client_address}")
            except socket.timeout:
                print("no client")
            time.sleep(1.0)

    def _recv_data(self):
        while self.is_running:
            if self.client_socket: # find function to see if there is incoming data
                try:
                    data = self.client_socket.recv(1024)
                    if not data:
                        break
                    numbers = struct.unpack("<" + "i" * (len(data) // 4), data)
                    self.plot_data = np.concatenate((self.plot_data, numbers))
                    self.plot_data = self.plot_data[-self.numpoints:]
                except socket.timeout:
                   # No data received from client within timeout period, reset client
                   self.client_socket = None
                   print("\nno data recieved timout.")
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

    def update_plot(self):
        self.ax.clear()
        self.ax.plot(self.plot_data)
        plt.pause(0.01)

    def _handle_close(self, evt):
        self.stop()
        plt.close()

if __name__ == "__main__":
    server = TCPServer()
    server.start()
    try:
        while True:
            server.update_plot()
    except KeyboardInterrupt:
        print("\nKeyboard interrupt received. Stopping server.")
        server.stop()
