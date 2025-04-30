import pygame
import json
import sys

# 初始化 Pygame
pygame.init()

# 設定視窗
WIDTH, HEIGHT = 1000, 400
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("玩家裝備顏色可視化")
clock = pygame.time.Clock()

# 顏色定義
BACKGROUND = (50, 50, 50)  # 深灰色背景
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)

# 載入玩家資料
def load_player_data():
    try:
        with open('output.txt', 'r', encoding='utf-8') as file:
            data = json.load(file)
            return data['players']
    except Exception as e:
        print(f"載入資料失敗: {e}")
        return None

# 繪製單個機器人
def draw_robot(surface, x, y, colors, scale=1.0):
    """ 繪製機器人
        colors: 包含各部位顏色的字典，格式如:
        {'head': (255,255,0), 'shoulder': (255,0,0), ...}
    """
    # 身體基礎位置
    body_width = int(40 * scale)
    body_height = int(80 * scale)
    
    # 繪製各部位 (使用get方法提供預設顏色)
    # 頭部 (圓形)
    pygame.draw.circle(surface, colors.get('head', WHITE), 
                      (x, y - int(50 * scale)), int(20 * scale))
    
    # 身體 (矩形)
    pygame.draw.rect(surface, colors.get('chest', WHITE), 
                    (x - body_width//2, y - body_height//2, body_width, body_height))
    
    # 肩膀
    shoulder_width = int(30 * scale)
    pygame.draw.rect(surface, colors.get('shoulder', WHITE), 
                    (x - int(50 * scale), y - int(20 * scale), shoulder_width, int(15 * scale)))
    pygame.draw.rect(surface, colors.get('shoulder', WHITE), 
                    (x + int(20 * scale), y - int(20 * scale), shoulder_width, int(15 * scale)))
    
    # 腿部
    leg_height = int(40 * scale)
    pygame.draw.rect(surface, colors.get('leg', WHITE), 
                    (x - int(15 * scale), y + body_height//2, int(15 * scale), leg_height))
    pygame.draw.rect(surface, colors.get('leg', WHITE), 
                    (x, y + body_height//2, int(15 * scale), leg_height))
    
    # 武器 (簡化為小矩形)
    pygame.draw.rect(surface, colors.get('weap_1', WHITE), 
                    (x - int(40 * scale), y, int(10 * scale), int(10 * scale)))
    pygame.draw.rect(surface, colors.get('weap_2', WHITE), 
                    (x + int(30 * scale), y, int(10 * scale), int(10 * scale)))

# 主程式
def main():
    players = load_player_data()
    if not players:
        print("無法載入玩家資料，使用預設值")
        players = [{}]  # 使用空字典作為預設
    
    selected_player = 0  # 預設顯示玩家0
    selected_time = 0    # 預設顯示時間點0
    
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_LEFT:
                    selected_player = max(0, selected_player - 1)
                elif event.key == pygame.K_RIGHT:
                    selected_player = min(len(players)-1, selected_player + 1)
                elif event.key == pygame.K_UP:
                    selected_time = min(len(players[selected_player])-1, selected_time + 1)
                elif event.key == pygame.K_DOWN:
                    selected_time = max(0, selected_time - 1)
        
        # 清空畫面
        screen.fill(BACKGROUND)
        
        # 取得當前要顯示的玩家資料
        try:
            time_data = players[selected_player][selected_time]
        except IndexError:
            time_data = {}  # 如果時間點不存在，使用空字典
        
        # 將數值轉換為顏色 (簡單的映射範例)
        def value_to_color(value):
            if value == 100: return BLACK  # 預設值顯示黑色
            # 簡單的顏色映射，可以根據需要修改
            return (
                (value >> 16) & 0xFF,  # R
                (value >> 8) & 0xFF,    # G
                value & 0xFF            # B
            )
        
        # 準備顏色字典
        colors = {
            'head': value_to_color(time_data.get('head', 100)),
            'shoulder': value_to_color(time_data.get('shoulder', 100)),
            'chest': value_to_color(time_data.get('chest', 100)),
            'front': value_to_color(time_data.get('front', 100)),
            'skirt': value_to_color(time_data.get('skirt', 100)),
            'leg': value_to_color(time_data.get('leg', 100)),
            'shoes': value_to_color(time_data.get('shoes', 100)),
            'weap_1': value_to_color(time_data.get('weap_1', 100)),
            'weap_2': value_to_color(time_data.get('weap_2', 100)),
        }
        
        # 繪製機器人 (居中)
        draw_robot(screen, WIDTH//2, HEIGHT//2, colors, scale=1.5)
        
        # 顯示操作說明
        font = pygame.font.SysFont(None, 24)
        text = font.render(f"玩家: {selected_player} 時間點: {selected_time} (使用方向鍵切換)", True, WHITE)
        screen.blit(text, (20, 20))
        
        pygame.display.flip()
        clock.tick(30)
    
    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()