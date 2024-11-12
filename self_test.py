from othello.bots.Random import BOT as black_bot
from othello.bots.Alpha_Beta import BOT as white_bot
from othello.OthelloGame import OthelloGame

black=black_bot() # 建立隨機bot
white=white_bot() # 建立Alpha-Beta bot

# 進行對局
game=OthelloGame(8)
game.play(white, black)
