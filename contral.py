import pygame
import socket
import time
import threading
import struct

# 初始化 Pygame
pygame.init()

# 畫面大小與顏色設置
WIDTH, HEIGHT = 800, 600
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
GRAY = (200, 200, 200)
BLUE = (0, 122, 255)
RED = (255, 0, 0)

# 創建顯示視窗
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Device Monitor")

# 字型設置
font = pygame.font.SysFont("Times New Roman", 20)

# 設備狀態存儲
devices = {}
exit_event = threading.Event()  # 新增結束程式的事件
stop_event = threading.Event()  # 用於控制停止功能的事件
start_event = threading.Event()  # 控制開始功能
heartbeat_event = threading.Event()
heartbeat_event.set()

# UDP 通信相關設置
broadcast_address = "192.168.0.255"
port = 12345
response_port = 12346
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.bind(("", response_port))
exit_event = threading.Event()
current_broadcast_message = ""

def get_local_ip():
    temp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    temp_sock.connect(("8.8.8.8", 80))
    local_ip = temp_sock.getsockname()[0]
    temp_sock.close()
    return local_ip

local_ip = get_local_ip()

print("Computer IP:", local_ip)

# 定義每塊板子的狀態
class DeviceState:
    def __init__(self, ip, device_id):
        self.ip = ip
        self.device_id = device_id
        self.last_response_time = None
        self.status = "Disconnected"  # 初始狀態為未連線
        self.task_status = "Waiting"  # 初始任務狀態為等待中

# 按鈕類
class Button:
    def __init__(self, x, y, width, height, color, text, text_color, action):
        self.rect = pygame.Rect(x, y, width, height)
        self.color = color
        self.text = text
        self.text_color = text_color
        self.action = action
        self.enabled = True  # 新增屬性，預設按鈕啟用

    def draw(self, screen):
        # 創建一個新的 Surface 來繪製按鈕
        button_surface = pygame.Surface((self.rect.width, self.rect.height), pygame.SRCALPHA)

        # 根據按鈕狀態設置顏色和透明度
        if self.enabled:
            button_surface.fill(self.color)
        else:
            # 禁用狀態下設置透明效果
            button_surface.fill((*self.color[:3], 128))  # 顏色 + Alpha (透明度)

        # 將按鈕繪製到主畫布
        screen.blit(button_surface, (self.rect.x, self.rect.y))

        # 繪製按鈕文字
        text_surface = font.render(self.text, True, self.text_color)
        text_rect = text_surface.get_rect(center=self.rect.center)
        screen.blit(text_surface, text_rect)

    def is_clicked(self, pos):
        return self.enabled and self.rect.collidepoint(pos)  # 只有啟用時才可以點擊

# 按鈕動作

def broadcast_message(message):
    global current_broadcast_message
    current_broadcast_message = message 
    sock.sendto(message.encode(), (broadcast_address, port))
    if message != "heartbeat":
        print(f"Broadcasted message: {message}")

count = 0
rootTime = 0
lastTime = 0
isRunning = False
heartbeatTrig = True

def start_music():
    music_file = r"C:\Users\morga\Desktop\lightdance V2.mp3"  # 替換為你的音樂檔案路徑
    # time.sleep(3)  # 音樂出現的延遲
    play_music(music_file)

def play_music(file_path):
    pygame.mixer.init()
    pygame.mixer.music.load(file_path)
    pygame.mixer.music.play()

def stop_music():
    pygame.mixer.music.stop()

# 發送停止訊號，直到所有設備回應
def start_function():
    start_music() 
    time.sleep(0.5)  # 延遲調整
    broadcast_message("start")
    global isRunning
    global rootTime
    global count
    global heartbeatTrig
    isRunning = True
    rootTime = time.time()*1000
    count = 0

    heartbeatTrig = False

    # 禁用 Start 按鈕
    for button in buttons:
        if button.text == "Start":
            button.enabled = False

    # time.sleep(3)     #延遲調整

    # start_event.set()  # 啟動停止功能

    # def broadcast_start():
    #     while start_event.is_set():
    #         broadcast_message("start")
    #         time.sleep(0.1)  # 增加間隔時間以避免 CPU 過載

    # threading.Thread(target=broadcast_start, daemon=True).start()

    # while start_event.is_set():
    #     # 檢查是否所有設備都已回應 "stopped"
    #     all_started = all(device.task_status == "running" for device in devices.values())

    #     # 如果所有設備已停止，結束廣播
    #     if all_started:
    #         print("All devices have started.")
    #         rootTime = time.time()*1000
    #         isRunning = True
    #         start_event.clear()

# 發送停止訊號，直到所有設備回應
def stop_function():
    broadcast_message("stop")
    global isRunning
    global rootTime
    global heartbeatTrig

    isRunning = False
    rootTime = time.time()*1000    # stop_event.set()  # 啟動停止功能

    heartbeatTrig = True

    # 啟用 Start 按鈕
    for button in buttons:
        if button.text == "Start":
            button.enabled = True

    stop_music()


    # def broadcast_stop():
    #     while stop_event.is_set():
    #         broadcast_message("stop")
    #         time.sleep(0.01)  # 增加間隔時間以避免 CPU 過載

    # threading.Thread(target=broadcast_stop, daemon=True).start()

    # while stop_event.is_set():
    #     # 檢查是否所有設備都已回應 "stopped"
    #     all_stopped = all(device.task_status == "stopped" for device in devices.values())

    #     # 如果所有設備已停止，結束廣播
    #     if all_stopped:
    #         print("All devices have stopped.")
    #         isRunning = False
    #         stop_event.clear()


def exit_action():
    exit_event.set()
    pygame.quit()
    exit()

# 定期發送心跳訊號
def heartbeat_function():
    while not exit_event.is_set():  # 只要主程式未結束，執行緒就一直運行
        if heartbeatTrig:           # 根據 heartbeatTrig 決定是否發送心跳
            broadcast_message("heartbeat")
        time.sleep(0.1)          # 每 0.1 秒檢查一次


# 設置按鈕
buttons = [
    Button(50, 500, 100, 50, BLUE, "Start", WHITE, start_function),
    Button(200, 500, 100, 50, RED, "Stop", WHITE, stop_function),
    Button(350, 500, 100, 50, GRAY, "Exit", BLACK, exit_action),
]

# 獲取本機 IP 地址
def get_local_ip():
    temp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    temp_sock.connect(("8.8.8.8", 80))
    local_ip = temp_sock.getsockname()[0]
    temp_sock.close()
    return local_ip

local_ip = get_local_ip()

running = 0
# 接收設備回應的執行緒
def listen_for_responses():
    global running
    while not exit_event.is_set():
        try:
            data, addr = sock.recvfrom(1024)
            message = data.decode()
            device_ip = addr[0]

            if ":" in message:
                device_id, task_status = map(str.strip, message.split(":", 1))
            else:
                device_id, task_status = "Unknown", message

            if task_status == "running":
                running = 1
            elif task_status == "stopped":
                running = 0
            # 更新板子狀態
            if device_ip not in devices:
                devices[device_ip] = DeviceState(device_ip, device_id)
            devices[device_ip].last_response_time = time.time()
            devices[device_ip].status = (
                "Running" if running else "Connected"
            )
            devices[device_ip].task_status = task_status

        except Exception:
            pass

# 啟動回應監聽執行緒
listener_thread = threading.Thread(target=listen_for_responses, daemon=True)
listener_thread.start()

# 主循環
heartbeat_thread = threading.Thread(target=heartbeat_function, daemon=True)  # 啟動心跳執行緒
heartbeat_thread.start()

first = 1
code_running = True
while code_running:
    screen.fill(WHITE)

    # 在畫面上方顯示當前廣播的內容
    broadcast_text = font.render(f"Current Broadcast: {current_broadcast_message}", True, BLACK)
    screen.blit(broadcast_text, (50, 20))

    # 處理事件
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            code_running = False
            exit_event.set()  # 設定退出事件
        elif event.type == pygame.MOUSEBUTTONDOWN:
            for button in buttons:
                if button.is_clicked(event.pos):
                    button.action()  # 觸發按鈕行為

    # 繪製按鈕
    for button in buttons:
        button.draw(screen)

    current_time = time.time()
    # 更新裝置狀態
    y_offset = 50
    for ip, device in devices.items():
        # 判斷連線狀態
        if device.last_response_time and current_time - device.last_response_time > 2:
            device.status = "Disconnected"

        last_seen = (
            f"{current_time - device.last_response_time:.1f} seconds ago"
            if device.last_response_time
            else "Never"
        )
        if first == 1 or device.status != {'heartbeat received'}:
            first = 0
            status_text = f"Device ID: {device.device_id}, IP: {device.ip}, Status: {device.status}, Last Seen: {last_seen}"
        
        status_surface = font.render(status_text, True, BLACK)
        screen.blit(status_surface, (50, y_offset))
        y_offset += 30

    pygame.display.flip()
    time.sleep(0.005)  # 控制刷新速度

    if isRunning == True:
        currentTime = time.time()*1000
        if currentTime - rootTime >= 1000*count:
            count += 1
            # lastTime = currentTime
            number = currentTime - rootTime 
            number = int(number)
            data = struct.pack("!I", number)  # "!G" 表示網路字節序（大端）和無符號 32-bit 整數
            current_broadcast_message = number
            sock.sendto(data, (broadcast_address, port))
            print(f"Broadcasted number:{number}")

pygame.quit()
