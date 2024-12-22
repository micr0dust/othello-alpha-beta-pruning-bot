from AIGamePlatform import Othello
from othello.bots.alpha_beta import BOT

app=Othello() # 和平台建立WebSocket連線
bot=BOT() # 建立隨機bot

@app.competition(competition_id='test_6x6_1') # 競賽ID
def _callback_(board, color): # 當需要走步會收到盤面及我方棋種
    return bot.getAction(board, color, 9) # bot回傳落子座標
