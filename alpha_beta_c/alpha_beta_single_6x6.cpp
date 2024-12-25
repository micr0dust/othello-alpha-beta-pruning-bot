/*
 █████╗ ██╗     ██████╗ ██╗  ██╗ █████╗       ██████╗ ███████╗████████╗ █████╗ 
██╔══██╗██║     ██╔══██╗██║  ██║██╔══██╗      ██╔══██╗██╔════╝╚══██╔══╝██╔══██╗
███████║██║     ██████╔╝███████║███████║█████╗██████╔╝█████╗     ██║   ███████║
██╔══██║██║     ██╔═══╝ ██╔══██║██╔══██║╚════╝██╔══██╗██╔══╝     ██║   ██╔══██║
██║  ██║███████╗██║     ██║  ██║██║  ██║      ██████╔╝███████╗   ██║   ██║  ██║
╚═╝  ╚═╝╚══════╝╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝      ╚═════╝ ╚══════╝   ╚═╝   ╚═╝  ╚═╝
                                                                               
                 ██████╗ ████████╗██╗  ██╗███████╗██╗     ██╗      ██████╗ 
                ██╔═══██╗╚══██╔══╝██║  ██║██╔════╝██║     ██║     ██╔═══██╗
                ██║   ██║   ██║   ███████║█████╗  ██║     ██║     ██║   ██║
                ██║   ██║   ██║   ██╔══██║██╔══╝  ██║     ██║     ██║   ██║
                ╚██████╔╝   ██║   ██║  ██║███████╗███████╗███████╗╚██████╔╝
                 ╚═════╝    ╚═╝   ╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝ ╚═════╝ 
                                            made by Microdust (Basic version)
*/
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <set>
#include <array>
#include <thread>
#include <future>
#include <iostream>

using namespace::std;

#define WHITE 1
#define BLACK -1
#define CORNOR 20
#define STABLE 1
#define N 6  // 定義棋盤大小
#define NN 36
#define MAX_SCORE (STABLE * NN + CORNOR * 4 + NN)

extern "C"
{
    constexpr int WEIGHTS[N][N] = {
        {100, -20, 10, 10, -20, 100},
        {-20, -50, -2, -2, -50, -20},
        { 10,  -2,  5,  5,  -2,  10},
        { 10,  -2,  5,  5,  -2,  10},
        {-20, -50, -2, -2, -50, -20},
        {100, -20, 10, 10, -20, 100}
    };

    int heuristic_score(pair<int, int> move, const int board[N][N]) {
        return WEIGHTS[move.first][move.second];
    }

    struct Game
    {
        int board[N][N];
    };

    const vector<pair<int, int>> directions {{1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}};

    vector<pair<int, int>> getValidMoves(Game game, int color)
    {
        set<pair<int, int>> moves;
        for (int y = 0; y < N; ++y)
        for (int x = 0; x < N; ++x)
            if (game.board[y][x] == color)
                for (auto direction : directions)
                {
                    vector<pair<int, int>> flips;
                    for (int size = 1; size < N; ++size)
                    {
                        int ydir = y + direction.second * size;
                        int xdir = x + direction.first * size;
                        if (xdir >= 0 && xdir < N && ydir >= 0 && ydir < N)
                        {
                            if (game.board[ydir][xdir] == -color)
                                flips.emplace_back(ydir, xdir);
                            else if (game.board[ydir][xdir] == 0){
                                if (!flips.empty())
                                    moves.emplace(ydir, xdir);
                                break;
                            }
                            else break;
                        }
                        else break;
                    }
                }
        vector<pair<int, int>> validMoves(moves.begin(), moves.end());
        sort(validMoves.begin(), validMoves.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
            return heuristic_score(a, game.board) > heuristic_score(b, game.board);
        });

        return validMoves;
    }

    bool isValidMove(Game game, int color, pair<int, int> position)
    {
        auto valids = getValidMoves(game, color);
        return find(valids.begin(), valids.end(), position) != valids.end();
    }

    void executeMove(Game &game, int color, pair<int, int> position)
    {
        int y = position.first;
        int x = position.second;
        game.board[y][x] = color;
        for (auto direction : directions)
        {
            vector<pair<int, int>> flips;
            bool valid_route = false;
            for (int size = 1; size < N; ++size)
            {
                int ydir = y + direction.second * size;
                int xdir = x + direction.first * size;
                if (xdir >= 0 && xdir < N && ydir >= 0 && ydir < N)
                {
                    if (game.board[ydir][xdir] == -color)
                        flips.emplace_back(ydir, xdir);
                    else if (game.board[ydir][xdir] == color)
                    {
                        if (!flips.empty())
                            valid_route = true;
                        break;
                    }
                    else break;
                }
                else break;
            }
            if (valid_route)
                for (auto [fy, fx] : flips)
                    game.board[fy][fx] = color;
        }
    }

    bool isEndGame(Game game)
    {
        auto white_valid_moves = getValidMoves(game, WHITE).size();
        auto black_valid_moves = getValidMoves(game, BLACK).size();
        return white_valid_moves == 0 && black_valid_moves == 0;
    }

    class BOT
    {
    public:
        BOT() {}

        int stable_stones(Game game, int color)
        {
            int stable_stones = 0;

            auto is_stable = [&](int x, int y, int color)
            {
                if ((x == 0 && y == 0) ||
                    (x == 0 && y == N-1) ||
                    (x == N-1 && y == 0) ||
                    (x == N-1 && y == N-1))
                    return true;

                for (auto [dx, dy] : directions)
                {
                    int nx = x + dx, ny = y + dy;
                    bool stable_in_direction = true;
                    while (0 <= nx && nx < N && 0 <= ny && ny < N)
                    {
                        if (game.board[nx][ny] != color)
                        {
                            stable_in_direction = false;
                            break;
                        }
                        nx += dx;
                        ny += dy;
                    }
                    if (stable_in_direction)
                        return true;
                }
                return false;
            };

            for (int i = 0; i < N; ++i)
                for (int j = 0; j < N; ++j)
                    if (game.board[i][j] == color && is_stable(i, j, color))
                    {
                        stable_stones += STABLE;
                        if ((i == 0 && j == 0) ||
                            (i == 0 && j == N-1) ||
                            (i == N-1 && j == 0) ||
                            (i == N-1 && j == N-1))
                            stable_stones += CORNOR;
                    }

            return stable_stones;
        }

        double evaluate(Game game, int color)
        {
            int actions = getValidMoves(game, color).size();
            return (actions + stable_stones(game, color)) / static_cast<double>(MAX_SCORE);
        }

        double endgame_evaluation(Game game, int color)
        {
            int white_count = 0, black_count = 0;
                for (int i = 0; i < N; ++i)
                    for (int j = 0; j < N; ++j)
                    {
                        if (game.board[i][j] == WHITE)
                            white_count++;
                        if (game.board[i][j] == BLACK)
                            black_count++;
                    }
                if (color == WHITE && white_count <= black_count)
                    return ((NN - black_count)*(NN - black_count)) / static_cast<double>(NN * NN * NN);
                if (color == BLACK && black_count <= white_count)
                    return ((NN - white_count)*(NN - white_count)) / static_cast<double>(NN * NN * NN);
                if (color == WHITE)
                    return ((NN - black_count)*(NN - black_count)) / static_cast<double>(NN * NN);
                return ((NN - white_count)*(NN - white_count)) / static_cast<double>(NN * NN);
        }

        double max_value(Game game, int color, double alpha, double beta, int depth)
        {
            if (isEndGame(game))
                return endgame_evaluation(game, color);
            else if (depth == 0)
                return evaluate(game, color);
            auto valids = getValidMoves(game, color);
            if (valids.empty())
                return evaluate(game, color);
            for (auto position : valids)
            {
                Game game_copy = game;
                executeMove(game_copy, color, position);
                double score = min_value(game_copy, -color, alpha, beta, depth - 1);

                alpha = max(alpha, score);
                if (beta <= alpha)
                    break;
            }
            return alpha;
        }

        double min_value(Game game, int color, double alpha, double beta, int depth)
        {
            if (isEndGame(game))
                return endgame_evaluation(game, -color);
            else if (depth == 0)
                return evaluate(game, -color);
            auto valids = getValidMoves(game, color);
            if (valids.empty())
                return evaluate(game, color);
            for (auto position : valids)
            {
                Game game_copy = game;
                executeMove(game_copy, color, position);
                double score = max_value(game_copy, -color, alpha, beta, depth - 1);
                
                beta = min(beta, score);
                if (beta <= alpha)
                    break;
            }
            return beta;
        }

        int alpha_beta_search(Game game, int color, int depth)
        {
            double best_score = -numeric_limits<double>::infinity();
            auto valids = getValidMoves(game, color);
            int best_action = -1;

            for (auto position : valids)
            {
                Game game_copy = game;
                executeMove(game_copy, color, position);
                
                // 重新初始化 alpha 和 beta
                double alpha = -numeric_limits<double>::infinity();
                double beta = numeric_limits<double>::infinity();
                
                double score = min_value(game_copy, -color, alpha, beta, depth - 1);
                
                if (score > best_score)
                {
                    best_score = score;
                    best_action = position.first * N + position.second;
                }
            }

            return best_action;
        }
    };

    BOT *create_bot()
    {
        return new BOT();
    }

    void destroy_bot(BOT *bot)
    {
        delete bot;
    }

    int get_action(BOT *bot, Game game, int color, int depth=5)
    {
        double score = bot->evaluate(game, color);
        cout << "Score: " << score << endl;
        return bot->alpha_beta_search(game, color, depth);
    }
}

// g++ -shared -o alpha_beta_multi_thread_6x6.dll -fPIC alpha_beta_multi_thread_6x6.cpp