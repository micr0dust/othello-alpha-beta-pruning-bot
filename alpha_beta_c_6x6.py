import ctypes
import numpy as np
import os
import time
from AIGamePlatform import Othello

app = Othello()

WHITE = 1
BLACK = -1

N = 6  # 棋盤大小

# 定義 C++ Structure 和函數
class Game(ctypes.Structure):
    _fields_ = [("board", ctypes.c_int * N * N)]

# 獲取當前目錄
current_dir = os.path.dirname(os.path.abspath(__file__))

# 添加 MinGW-w64 的 bin 目錄到 DLL 搜索路徑
mingw_bin_path = r'C:\Program Files\mingw64\x86_64-14.2.0-release-posix-seh-ucrt-rt_v12-rev0\mingw64\bin'
os.add_dll_directory(mingw_bin_path)


os.add_dll_directory(current_dir) # 添加當前目錄到 DLL 搜索路徑
lib_dir = os.path.join(current_dir, 'lib') # 添加 lib 目錄到 DLL 搜索路徑
os.add_dll_directory(lib_dir)

# 確認 DLL 的存在
dll_path = os.path.join(lib_dir, 'alpha_beta_bit_6x6_ratio2.dll')
if not os.path.exists(dll_path):
    raise FileNotFoundError(f"Could not find the DLL: {dll_path}")

# 載入共享庫
alphabeta = ctypes.CDLL(dll_path)

# 定義函數參數和返回類型
alphabeta.create_bot.restype = ctypes.POINTER(ctypes.c_void_p)
alphabeta.get_action.argtypes = [ctypes.POINTER(ctypes.c_void_p), Game, ctypes.c_int]
alphabeta.get_action.restype = ctypes.c_int
alphabeta.destroy_bot.argtypes = [ctypes.POINTER(ctypes.c_void_p)]

# 創建 BOT
bot = alphabeta.create_bot()

# 定義遊戲狀態
game = Game()

last_cell = 100
begin = time.time()

def round_trigger(now_cells):
    global last_cell, begin
    if last_cell > now_cells:
        print(f"Time: {time.time() - begin}")
    last_cell = now_cells

# test_robot_6x6_1
@app.competition(competition_id='test_6x6_2')  # 競賽ID
def _callback_(board, color):  # 當需要走步會收到盤面及我方棋種
    def get_depth(now_cells): # 動態深度展開
        if now_cells == 4:
            return 1
        # if now_cells <= 36-20:
        #     return 13  # 中局
        return 10  # 殘局
    
    # 將傳入的 board 轉換為 ctypes 數組
    board_array = np.array(board, dtype=np.int32).flatten()
    ctypes_array = (ctypes.c_int * (N * N))(*board_array)
    ctypes.memmove(game.board, ctypes_array, ctypes.sizeof(ctypes_array))
    
    now_cells = N * N - np.sum(board == 0)
    round_trigger(now_cells)
        
    # bot回傳落子座標
    res = alphabeta.get_action(bot, game, color, get_depth(now_cells))
    x, y = divmod(res, N)
    return (x, y)