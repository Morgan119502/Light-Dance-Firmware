import json
import numpy as np
import requests

# 設定目標網址
url = "http://140.113.160.136:8000/items/eesa1/LATEST"

try:
    # 發送 GET 請求
    response = requests.get(url)
    
    # 檢查請求是否成功 (HTTP 狀態碼 200 表示成功)
    if response.status_code == 200:
        # 將內容寫入檔案 (預設存為 output.txt)
        with open("output.txt", "w", encoding="utf-8") as file:
            file.write(response.text)
        print("下載成功！內容已保存到 output.txt")
    else:
        print(f"下載失敗，HTTP 狀態碼: {response.status_code}")

except requests.exceptions.RequestException as e:
    print(f"發生錯誤: {e}")

def load_player_data():
    try:
        with open('output.txt', 'r', encoding='utf-8') as file:
            data = json.load(file)
            
            players = data.get('players', [])
            if not players:
                raise ValueError("No player data found in JSON")
            
            # 動態計算維度大小
            player_count = len(players)
            time_points = max(len(player) for player in players) if player_count > 0 else 0
            equipment_slots = 10  # 10個裝備欄位
            
            # 初始化陣列時使用動態計算的維度
            array = np.zeros((player_count, time_points, equipment_slots), dtype=np.uint32)
            
            for i, player in enumerate(players):
                actual_time_points = len(player)  # 當前玩家的實際時間點數量
                for j in range(actual_time_points):  # 只遍歷實際存在的時間點
                    time_data = player[j]
                    equipment_values = [
                        time_data.get('head', 0),
                        time_data.get('shoulder', 0),
                        time_data.get('chest', 0),
                        time_data.get('front', 0),
                        time_data.get('skirt', 0),
                        time_data.get('leg', 0),
                        time_data.get('shoes', 0),
                        time_data.get('weap_1', 0),
                        time_data.get('weap_2', 0),
                        0  # 預留位置
                    ]
                    array[i,j] = equipment_values[:equipment_slots]
            
            print(f"成功載入 {player_count} 位玩家資料，最大時間點數: {time_points}")
            return array
            
    except FileNotFoundError:
        print("錯誤：找不到 output.txt 檔案")
    except json.JSONDecodeError:
        print("錯誤：檔案不是有效的 JSON 格式")
    except Exception as e:
        print(f"發生錯誤：{str(e)}")
    return None

if __name__ == "__main__":
    player_array = load_player_data()
    if player_array is not None:
        print("陣列維度:", player_array.shape)
        # 顯示有效範圍內的資料
        print("玩家0最後時間點資料:")
        for i in range(184):
            print(player_array[0, i])