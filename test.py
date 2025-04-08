import socket

def get_local_ip():
    try:
        # 创建一个临时 UDP 套接字（不发送数据）
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.connect(("8.8.8.8", 80))  # 连接 Google DNS
        local_ip = sock.getsockname()[0]  # 获取本地 IP
        sock.close()
        return local_ip
    except Exception as e:
        print("获取 IP 失败:", e)
        return "127.0.0.1"  # 失败时返回本地回环地址

print("本机 IP:", get_local_ip())