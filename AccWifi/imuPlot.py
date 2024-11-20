import socket
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# Configuration
HOST = '192.168.4.1'  # IP address of the M5StickC Plus (Wi-Fi AP)
PORT = 8080           # Port for the TCP server

def main():
    try:
        # Create a socket to connect to the server
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print(f"Connecting to {HOST}:{PORT}...")
        client_socket.connect((HOST, PORT))
        print("Connected to the server!")
        
        # Prepare for plotting
        plt.style.use('ggplot')
        fig, ax = plt.subplots()
        line_x, = ax.plot([], [], label='X-axis')
        line_y, = ax.plot([], [], label='Y-axis')
        line_z, = ax.plot([], [], label='Z-axis')
        ax.legend()
        ax.set_xlabel('Time')
        ax.set_ylabel('Acceleration')
        ax.set_title('Real-Time Accelerometer Data')

        # Initialize data storage
        time_data = deque(maxlen=1000)
        x_data = deque(maxlen=1000)
        y_data = deque(maxlen=1000)
        z_data = deque(maxlen=1000)

        # Buffer to store incomplete data
        buffer = ''

        def update_plot(frame):
            nonlocal buffer
            try:
                data = client_socket.recv(1024)  # Receive up to 1024 bytes
                if not data:
                    print("Connection closed by the server.")
                    plt.close(fig)
                    return

                buffer += data.decode('utf-8')
                lines = buffer.split('\n')
                buffer = lines[-1]  # Save the last incomplete line
                for line in lines[:-1]:
                    # Process each complete line
                    try:
                        t_str, x_str, y_str, z_str = line.strip().split(',')
                        t = float(t_str)
                        x = float(x_str)
                        y = float(y_str)
                        z = float(z_str)
                        time_data.append(t)
                        x_data.append(x)
                        y_data.append(y)
                        z_data.append(z)
                    except ValueError:
                        # Handle incomplete or invalid lines
                        continue

                # Update plot data
                line_x.set_data(time_data, x_data)
                line_y.set_data(time_data, y_data)
                line_z.set_data(time_data, z_data)
                ax.relim()
                ax.autoscale_view()

                return line_x, line_y, line_z
            except Exception as e:
                print(f"An error occurred: {e}")
                plt.close(fig)

        ani = animation.FuncAnimation(fig, update_plot, interval=50)
        plt.show()

    except ConnectionRefusedError:
        print(f"Failed to connect to {HOST}:{PORT}. Ensure the server is running.")
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        client_socket.close()
        print("Connection closed.")

if __name__ == "__main__":
    main()
