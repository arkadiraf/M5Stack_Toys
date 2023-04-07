import socket
import threading
import time
import struct
import matplotlib.pyplot as plt
import wave

n_channels = 1  # Mono
sample_width = 2  # 16 bits
sample_rate = 44100  # 44.1 kHz
file_name = "samples.wav"


class TCPServer:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.client_socket = None
        self.is_running = False
        self.fig, self.ax = plt.subplots()
        self.plot_data = []
        self.start_time = None
        self.wav_file = None

    def start(self):
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen()
        print(f"Listening on {self.host}:{self.port}")
        self.is_running = True
        self.start_time = time.time()  # record start time
        self.wav_file = wave.open(file_name, mode="wb")
        self.wav_file.setnchannels(n_channels)
        self.wav_file.setsampwidth(sample_width)
        self.wav_file.setframerate(sample_rate)

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
        self.wav_file.close()
        print("WAV file saved:", file_name)

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
                    numbers = struct.unpack("<" + "h" * (len(data) // 2), data)
                    self.plot_data.extend(numbers)
                    packed_data = struct.pack(f"<{len(numbers)}h", *numbers)
                    self.wav_file.writeframes(data)
                except ConnectionResetError:
                    print("\nConnection with client reset.")
                    self.client_socket = None
            elapsed_time = time.time() - self.start_time  # compute elapsed time
            if elapsed_time > 10:
                print("\n 10 seconds have passed")
                self.stop()

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
        plt.pause(0.001)


if __name__ == "__main__":
    HOST = "192.168.0.105"  # The server's hostname or IP address
    PORT = 65432 
