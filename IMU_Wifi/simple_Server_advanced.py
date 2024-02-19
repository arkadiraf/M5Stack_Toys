import socket
import threading
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import numpy as np
import datetime

# Use a style for the plot
style.use('fivethirtyeight')

# TCP Server class definition
class TCPServer:
    def __init__(self, host='192.168.0.103', port=60691):
        self.host = host
        self.port = port
        self.serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.clientSockets = []
        self.running = True
        self.data = []

        # Generate a timestamped filename
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        self.filename = f"imu_data_{timestamp}.txt"
        
        # Open the file for writing
        self.file = open(self.filename, "w")

    def handle_client_connection(self, clientSocket, address):
        print(f"Accepted connection from {address}")
        buffer = ""
        while True:
            try:
                data = clientSocket.recv(1024)
                if not data:
                    break
                buffer += data.decode()
                while "\r\n" in buffer:
                    message, buffer = buffer.split("\r\n", 1)
                    try:
                        parts = message.split(':')
                        if len(parts) > 1:
                            imu_data = list(map(float, parts[1].split(',')))
                            self.data.append(imu_data)
                            # Write to the pre-opened file
                            self.file.write(message + "\n")
                    except ValueError as e:
                        print(f"Error processing data '{message}': {e}")
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
                # Close all client sockets
                for cs in self.clientSockets:
                    cs.close()
                # Close the server socket
                self.serverSocket.close()
                # Close the file
                self.file.close()
                print(f"Data recorded to {self.filename}")

def animate(i, server):
    # Ensure there's enough data to plot
    if len(server.data) > 0:
        data = np.array(server.data[-200:])  # Take the last 200 data points
        # Update the data for each line
        line1.set_data(data[:, 0], data[:, 1])  # Timestamp vs AccX
        line2.set_data(data[:, 0], data[:, 2])  # Timestamp vs AccY
        line3.set_data(data[:, 0], data[:, 3])  # Timestamp vs AccZ
        line4.set_data(data[:, 0], data[:, 7])  # Timestamp vs AccZ
        # Adjusting plot limits
        plt.xlim(np.min(data[:, 0]), np.max(data[:, 0]))  # Dynamically adjust x-axis limits
        plt.ylim(-3, 3)  # Fixed y-axis limits as per your specification

        # Update plot labels if needed
        plt.xlabel('Time')
        plt.ylabel('Acceleration (g)')
        plt.title('Real-time Acceleration Data')

if __name__ == '__main__':
    server = TCPServer()

    # Start the TCP server in a separate thread
    threading.Thread(target=server.start, daemon=True).start()
    
    fig, ax = plt.subplots()
    line1, = ax.plot([], [], lw=2, label='Acc X')
    line2, = ax.plot([], [], lw=2, label='Acc Y')
    line3, = ax.plot([], [], lw=2, label='Acc Z')
    line4, = ax.plot([], [], lw=2, label='Button')
    plt.legend()

    ani = animation.FuncAnimation(fig, animate, fargs=(server,), interval=10)

    plt.show()
