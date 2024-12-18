// alphabeta.cpp
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

    constexpr int STABLE_ORDER[] = {
        0, 1, 2, 3, 4, 5,
        11, 17, 23, 29, 35,
        34, 33, 32, 31, 30,
        24, 18, 12, 6, 7,
        8, 9, 10, 16, 22,
        28, 27, 26, 25, 19,
        13, 14, 15, 21, 20
    };

    int heuristic_score(pair<int, int> move, const int board[N][N]) {
        return WEIGHTS[move.first][move.second];
    }

    struct Game
    {
        int board[N][N];
    };

    vector<vector<int>> stable_board;

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

        int stable_stones(Game& game, vector<vector<int>>& stable_map, int color) {
            // 初始條件：先將角落視為穩定子
            const vector<pair<int, int>> corners = {{0, 0}, {0, N - 1}, {N - 1, 0}, {N - 1, N - 1}};
            for (auto [cx, cy] : corners)
                if (game.board[cx][cy] != 0)
                    stable_map[cx][cy] = game.board[cx][cy];
            // 從角落與邊界向內逐層檢查
            bool updated = true;
            while (updated) {
                updated = false; // 若沒有新穩定子，結束迴圈
                for (int idx = 0; idx < NN; ++idx) {
                    int x = STABLE_ORDER[idx] / N, y = STABLE_ORDER[idx] % N;
                    if (game.board[x][y] == 0) continue; // 跳過空格
                    if (stable_map[x][y]) continue; // 已經是穩定子
                    int team = game.board[x][y];
                    bool is_stable = true; // 檢查是否為穩定子
                    for (int i = 0; i < 4; ++i){
                        auto [dx, dy] = directions[i];
                        int nx = x + dx, ny = y + dy;
                        int mx = x - dx, my = y - dy;

                        if( // **方向穩定的條件**：
                            (nx < 0 || nx >= N || ny < 0 || ny >= N) || // 正向碰邊界
                            (mx < 0 || mx >= N || my < 0 || my >= N) || // 反向碰邊界
                            stable_map[nx][ny] == team || // 正向碰穩定子
                            stable_map[mx][my] == team || // 反向碰穩定子
                            (stable_map[nx][ny] == -team) && (stable_map[mx][my] == -team) // 在對手穩定子中間
                        ) continue;

                        is_stable = false; // 此方向不穩定，跳出檢查
                        break;
                    }

                    if (is_stable) {
                        stable_map[x][y] = game.board[x][y];
                        updated = true; // 更新時需要重新檢查
                    }
                }
            }

            int stable_count = 0;
            for(int i = 0; i < N; i++)
                for(int j = 0; j < N; j++)
                    if(stable_map[i][j] == color)
                        stable_count++;
            for (auto [cx, cy] : corners)
                if (stable_map[cx][cy] == color)
                    stable_count += CORNOR;
            return stable_count;
        }


        double evaluate(Game game, vector<vector<int>> stable_map, int color)
        {
            int actions = getValidMoves(game, color).size();
            return (actions + stable_stones(game, stable_map, color)) / static_cast<double>(MAX_SCORE);
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
                return evaluate(game, stable_board, color);
            auto valids = getValidMoves(game, color);
            if (valids.empty())
                return evaluate(game, stable_board, color);
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
                return evaluate(game, stable_board, -color);
            auto valids = getValidMoves(game, color);
            if (valids.empty())
                return evaluate(game, stable_board, color);
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

            vector<future<pair<double, int>>> futures;
            for (auto position : valids)
            {
                futures.push_back(
                    async(launch::async, 
                    [this, game, color, position, depth](){
                        Game game_copy = game;
                        executeMove(game_copy, color, position);
                        
                        // 重新初始化 alpha 和 beta
                        double alpha = -numeric_limits<double>::infinity();
                        double beta = numeric_limits<double>::infinity();
                        
                        double score = min_value(game_copy, -color, alpha, beta, depth - 1);
                        return make_pair(score, position.first * N + position.second);
                    }
                ));
            }

            for (auto &future : futures)
            {
                auto [score, action] = future.get();
                if (score > best_score)
                {
                    best_score = score;
                    best_action = action;
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
        stable_board = vector(N, vector<int>(N, 0));
        bot->stable_stones(game, stable_board, color);
        double score = bot->evaluate(game, stable_board, color);
        cout << "Score: " << score << endl;
        return bot->alpha_beta_search(game, color, depth);
    }

    int debug(BOT *bot, Game game, int color, int depth=5)
    {
        stable_board = vector(N, vector<int>(N, 0));
        int score = bot->stable_stones(game, stable_board, color);
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                cout << stable_board[i][j] << " \n"[j == N - 1];
        
        return score;
    }
}

// g++ -shared -o alpha_beta_mt_true_stable_6x6.dll -fPIC alpha_beta_mt_true_stable_6x6.cpp