import socket
import sys
import time
from stun import get_ip_info

def get_public_address():
    nat_type, external_ip, external_port = get_ip_info()
    return (external_ip, external_port)

def create_socket():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', 0))
    return sock

def main():
    if len(sys.argv) != 3:
        print("Usage: python p2p_ping_test.py <mode> <peer_address>")
        print("mode: server or client")
        print("peer_address: IP:PORT")
        sys.exit(1)

    mode = sys.argv[1]
    peer_addr = sys.argv[2].split(':')
    peer_ip = peer_addr[0]
    peer_port = int(peer_addr[1])

    sock = create_socket()
    public_ip, public_port = get_public_address()
    print(f"My public address: {public_ip}:{public_port}")

    if mode == "server":
        while True:
            try:
                data, addr = sock.recvfrom(1024)
                if data == b"ping":
                    print(f"Received ping from {addr}")
                    sock.sendto(b"pong", addr)
            except KeyboardInterrupt:
                break

    else:  # client mode
        while True:
            try:
                sock.sendto(b"ping", (peer_ip, peer_port))
                sock.settimeout(2)
                try:
                    data, addr = sock.recvfrom(1024)
                    if data == b"pong":
                        print(f"Received pong from {addr}")
                except socket.timeout:
                    print("Timeout - no response")
                time.sleep(1)
            except KeyboardInterrupt:
                break

if __name__ == "__main__":
    main()
