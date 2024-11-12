import ctypes
import numpy as np
import os

WHITE = 1
BLACK = -1

# 定義 C++ 結構體和函數
class Game(ctypes.Structure):
    _fields_ = [("board", ctypes.c_int * 8 * 8)]

# 獲取當前目錄
current_dir = os.path.dirname(os.path.abspath(__file__))

# 添加 MinGW-w64 的 bin 目錄到 DLL 搜索路徑
mingw_bin_path = r'C:\Program Files\mingw64\x86_64-14.2.0-release-posix-seh-ucrt-rt_v12-rev0\mingw64\bin'
os.add_dll_directory(mingw_bin_path)

# 添加當前目錄到 DLL 搜索路徑
os.add_dll_directory(current_dir)

# 確認 DLL 的存在
dll_path = os.path.join(current_dir, 'alpha_beta.dll')
if not os.path.exists(dll_path):
    raise FileNotFoundError(f"Could not find the DLL: {dll_path}")

# 載入共享庫
alphabeta = ctypes.CDLL(dll_path)

# 定義函數參數和返回類型
alphabeta.create_bot.restype = ctypes.POINTER(ctypes.c_void_p)
alphabeta.get_action.argtypes = [ctypes.POINTER(ctypes.c_void_p), Game, ctypes.c_int]
alphabeta.get_action.restype = ctypes.c_int
alphabeta.destroy_bot.argtypes = [ctypes.POINTER(ctypes.c_void_p)]

# 創建 BOT 實例
bot = alphabeta.create_bot()

# 定義遊戲狀態
game = Game()

# 將 numpy 數組轉換為 ctypes 數組
board_array = np.zeros((8, 8), dtype=np.int32).flatten()
ctypes_array = (ctypes.c_int * 64)(*board_array)
ctypes.memmove(game.board, ctypes_array, ctypes.sizeof(ctypes_array))

# 設定棋盤狀態（這裡僅為範例，請根據實際情況設置）
game.board[3][3] = WHITE
game.board[3][4] = BLACK
game.board[4][3] = BLACK
game.board[4][4] = WHITE

# 獲取動作
color = WHITE
action = alphabeta.get_action(bot, game, color)
print(f"Best action: {action}")

# 銷毀 BOT 實例
alphabeta.destroy_bot(bot)