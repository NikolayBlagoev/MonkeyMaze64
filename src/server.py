import socket
import time

HOST = 'localhost'
PORT = 31691

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()

    print(f'Server listening on {HOST}:{PORT}')

    # Accept incoming connections
    conn, addr = s.accept()
    print(f'Connected by {addr}')
    conn.setblocking(False)
    # Receive and send messages
    with conn:
        while True:
            time.sleep(2)
            data = "hi"
            conn.send(data.encode(encoding="UTF_8"))
            try:
                data = conn.recv(1024)
                if not data:
                    break 
            except Exception:
                continue 
            
            # print(f'Received: {data.decode()}')
            

    print(f'Connection closed by {addr}')