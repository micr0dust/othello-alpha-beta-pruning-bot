import time
import numpy as np
from AIGamePlatform import Othello
from othello.bots.alpha_beta import BOT

N = 6  # 棋盤大小

app=Othello() # 和平台建立WebSocket連線
bot=BOT() # 建立隨機bot

last_cell = 100
begin = time.time()

def round_trigger(now_cells):
    global last_cell, begin
    if last_cell > now_cells:
        print(f"Time: {time.time() - begin}")
    last_cell = now_cells

@app.competition(competition_id='test_robot_6x6_2') # 競賽ID
def _callback_(board, color): # 當需要走步會收到盤面及我方棋種
    now_cells = N * N - np.sum(board == 0)
    round_trigger(now_cells)
    return bot.getAction(board, color, 8) # bot回傳落子座標

