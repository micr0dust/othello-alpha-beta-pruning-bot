import numpy as np
from othello.OthelloUtil import getValidMoves, isValidMove, executeMove, isEndGame
from othello.OthelloGame import WHITE, BLACK
import multiprocessing
from multiprocessing import Pool
PROCESS = multiprocessing.cpu_count()

CORNOR = 10
STABLE = 1

def move(game, color, position):
    if isValidMove(game, color, position):
        executeMove(game, color, position)
    else:
        raise Exception('invalid move')
        
class BOT():
    def __init__(self, *args, **kargs):
        pass
    
    def stable_stones(self, game, color):
        stable_stones = 0
        directions = [(-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1)]
        
        def is_stable(x, y, color):
            if (x, y) in [(0, 0), (0, 7), (7, 0), (7, 7)]:
                return True
            
            for dx, dy in directions:
                nx, ny = x + dx, y + dy
                stable_in_direction = True
                while 0 <= nx < 8 and 0 <= ny < 8:
                    if game[nx][ny] != color:
                        stable_in_direction = False
                        break
                    nx += dx
                    ny += dy
                if stable_in_direction:
                    return True
            return False

        # 檢查棋盤上的每個位置
        for i in range(8):
            for j in range(8):
                if game[i][j] == color and is_stable(i, j, color):
                    stable_stones += STABLE
                    if (i, j) in [(0, 0), (0, 7), (7, 0), (7, 7)]:
                        stable_stones += CORNOR

        return stable_stones


    def evaluate(self, game, color):
        actions = len(getValidMoves(game, color))
        MAX_SCORE = STABLE*64 + CORNOR*4 + 64
        # v,c=np.unique(game, return_counts=True)
        return (actions \
                + self.stable_stones(game, color)\
                )/MAX_SCORE
    
    def endgame_evaluation(self, game, color):
        white_valid_moves=len(getValidMoves(game, WHITE))
        black_valid_moves=len(getValidMoves(game, BLACK))
        if white_valid_moves==0 and black_valid_moves==0:
            v,c=np.unique(game, return_counts=True)
            if color==WHITE:
                return (64-c[np.where(v==BLACK)][0])/64
            else:
                return (64-c[np.where(v==WHITE)][0])/64

    def max_value(self, game, color, alpha, beta, depth):
        if isEndGame(game):
            return self.endgame_evaluation(game, color)
        elif depth == 0:
            return self.evaluate(game, color)
        valids = getValidMoves(game, color)
        if len(valids) == 0:
            return self.evaluate(game, color)
        
        first = True
        for position in valids:
            game_copy = game.copy()
            move(game_copy, color, position)
            if first:  # scout 第一次搜尋時要先找到 alpha 和 beta，不用 test
                score = self.min_value(game_copy, -color, alpha, beta, depth - 1)
                first = False
            else:
                score = self.min_value(game_copy, -color, alpha + 1e-9, alpha + 1e-9, depth - 1)
                if score > alpha: # test
                    score = self.min_value(game_copy, -color, score, beta, depth - 1)
            if score >= beta:
                return beta
            alpha = max(alpha, score)
        return alpha

    def min_value(self, game, color, alpha, beta, depth):
        if isEndGame(game):
            return self.endgame_evaluation(game, -color)
        elif depth == 0:
            return self.evaluate(game, -color)
        valids = getValidMoves(game, color)
        if len(valids) == 0:
            return self.evaluate(game, color)
        
        first = True
        for position in valids:
            game_copy = game.copy()
            move(game_copy, color, position)
            if first: # scout 第一次搜尋時要先找到 alpha 和 beta，不用 test
                score = self.max_value(game_copy, -color, alpha, beta, depth - 1)
                first = False
            else:
                score = self.max_value(game_copy, -color, beta - 1e-9, beta - 1e-9, depth - 1)
                if score < beta: # test
                    score = self.max_value(game_copy, -color, alpha, score, depth - 1)
            if score <= alpha:
                return alpha
            beta = min(beta, score)
        return beta

    def evaluate_position(self, position, game, color, alpha, beta, depth):
        game_copy = game.copy()
        move(game_copy, color, position)
        score = self.max_value(game_copy, -color, alpha, beta, depth - 1)
        return (1-score, position)

    def alpha_beta_search(self, game, color, depth):
        best_score = -np.inf
        alpha = -np.inf
        beta = np.inf
        valids = getValidMoves(game, color)
        best_action = None

        MULTIPROCCEESS = True

        if MULTIPROCCEESS:
            with Pool(processes=PROCESS) as pool:
                futures = [pool.apply_async(self.evaluate_position, args=(position, game, color, alpha, beta, depth)) for position in valids]
                for future in futures:
                    score, position = future.get()
                    if score > best_score:
                        best_score = score
                        best_action = position
        else:
            for position in valids:
                score, position = self.evaluate_position(position, game, color, alpha, beta, depth)
                if score > best_score:
                    best_score=score
                    best_action=position

        return best_action


    def getAction(self, game, color):
        return self.alpha_beta_search(game, color, 5)