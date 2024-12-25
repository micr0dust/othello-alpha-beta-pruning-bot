import ctypes
import numpy as np
import os
import time
import configparser

# 建立 ConfigParser 物件
config = configparser.ConfigParser()

# 讀取設定檔
config.read('config.cfg')

WHITE = 1
BLACK = -1

N = 6  # 棋盤大小

# 定義 C++ Structure 和函數
class Game(ctypes.Structure):
    _fields_ = [("board", ctypes.c_int * N * N)]

# 獲取當前目錄
current_dir = os.path.dirname(os.path.abspath(__file__))

# 添加 MinGW-w64 的 bin 目錄到 DLL 搜索路徑
mingw_bin_path = config['Settings']['mingw_bin_path']
os.add_dll_directory(mingw_bin_path)


os.add_dll_directory(current_dir) # 添加當前目錄到 DLL 搜索路徑
lib_dir = os.path.join(current_dir, 'lib') # 添加 lib 目錄到 DLL 搜索路徑
os.add_dll_directory(lib_dir)

# 確認 DLL 的存在
dll_path = os.path.join(lib_dir, config['Settings']['dll'])
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
default_depth = config.getint('Settings', 'depth')
change_depth = default_depth
print_time = config.getboolean('verbose', 'timer')
print_depth = config.getboolean('verbose', 'depth')

def round_trigger(now_cells):
    global last_cell, begin, change_depth
    if last_cell > now_cells:
        if print_time:
            print(f"Time: {time.time() - begin}")
        try:
            config.read('config.cfg')
            change_depth = config.getint('Settings', 'depth')
        except FileNotFoundError:
            change_depth = default_depth
        if print_depth:
            print(f"depth: {change_depth}")
    last_cell = now_cells

def get_action(board, color):
    def get_depth(now_cells):
        if now_cells == 4: return 1
        if type(change_depth) == int and change_depth>0:
            return change_depth
        return default_depth
    
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

if __name__ == '__main__':
    # 測試
    board = np.zeros((N, N), dtype=np.int32)
    board[2][2] = WHITE
    board[3][3] = WHITE
    board[2][3] = BLACK
    board[3][2] = BLACK
    print(get_action(board, BLACK))  # (2, 1)