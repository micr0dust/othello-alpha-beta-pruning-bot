// alphabeta.cpp
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <set>
#include <array>
#include <thread>
#include <future>

using namespace::std;

#define WHITE 1
#define BLACK -1
#define CORNOR 10
#define STABLE 1
#define MAX_SCORE (STABLE * 64 + CORNOR * 4 + 64)

extern "C"
{

    struct Game
    {
        int board[8][8];
    };

    const vector<pair<int, int>> directions {{1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}};

    vector<pair<int, int>> getValidMoves(Game game, int color)
    {
        set<pair<int, int>> moves;
        for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            if (game.board[y][x] == color)
                for (auto direction : directions)
                {
                    vector<pair<int, int>> flips;
                    for (int size = 1; size < 8; ++size)
                    {
                        int ydir = y + direction.second * size;
                        int xdir = x + direction.first * size;
                        if (xdir >= 0 && xdir < 8 && ydir >= 0 && ydir < 8)
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
        return vector<pair<int, int>>(moves.begin(), moves.end());
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
            for (int size = 1; size < 8; ++size)
            {
                int ydir = y + direction.second * size;
                int xdir = x + direction.first * size;
                if (xdir >= 0 && xdir < 8 && ydir >= 0 && ydir < 8)
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
                    (x == 0 && y == 7) ||
                    (x == 7 && y == 0) ||
                    (x == 7 && y == 7))
                    return true;

                for (auto [dx, dy] : directions)
                {
                    int nx = x + dx, ny = y + dy;
                    bool stable_in_direction = true;
                    while (0 <= nx && nx < 8 && 0 <= ny && ny < 8)
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

            for (int i = 0; i < 8; ++i)
                for (int j = 0; j < 8; ++j)
                    if (game.board[i][j] == color && is_stable(i, j, color))
                    {
                        stable_stones += STABLE;
                        if ((i == 0 && j == 0) ||
                            (i == 0 && j == 7) ||
                            (i == 7 && j == 0) ||
                            (i == 7 && j == 7))
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
            int white_valid_moves = getValidMoves(game, WHITE).size();
            int black_valid_moves = getValidMoves(game, BLACK).size();
            if (white_valid_moves == 0 && black_valid_moves == 0)
            {
                int white_count = 0, black_count = 0;
                for (int i = 0; i < 8; ++i)
                    for (int j = 0; j < 8; ++j)
                    {
                        if (game.board[i][j] == WHITE)
                            white_count++;
                        if (game.board[i][j] == BLACK)
                            black_count++;
                    }
                if (color == WHITE)
                    return (64 - black_count) / 64.0;
                else
                    return (64 - white_count) / 64.0;
            }
            return 0;
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
            bool first = true;
            for (auto position : valids)
            {
                Game game_copy = game;
                executeMove(game_copy, color, position);
                double score;
                if(first){
                    first = false;
                    score = min_value(game_copy, -color, alpha, beta, depth - 1);
                }else{
                    score = min_value(game_copy, -color, alpha, alpha, depth - 1);
                    if (score > alpha)
                        score = min_value(game_copy, -color, alpha, beta, depth - 1);
                }
                if (score >= beta)
                    return beta;
                alpha = max(alpha, score);
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
            bool first = true;
            for (auto position : valids)
            {
                Game game_copy = game;
                executeMove(game_copy, color, position);
                double score;
                if(first){
                    first = false;
                    score = max_value(game_copy, -color, alpha, beta, depth - 1);
                }else{
                    score = max_value(game_copy, -color, beta, beta, depth - 1);
                    if (score < beta)
                        score = max_value(game_copy, -color, alpha, beta, depth - 1);
                }
                if (score <= alpha)
                    return alpha;
                beta = min(beta, score);
            }
            return beta;
        }

        int alpha_beta_search(Game game, int color, int depth)
        {
            double best_score = -numeric_limits<double>::infinity();
            double alpha = -numeric_limits<double>::infinity();
            double beta = numeric_limits<double>::infinity();
            auto valids = getValidMoves(game, color);
            int best_action = -1;

            vector<future<pair<double, int>>> futures;
            for (auto position : valids)
            {
                futures.push_back(
                    async(launch::async, 
                    [this, game, color, position, alpha, beta, depth](){
                        Game game_copy = game;
                        executeMove(game_copy, color, position);
                        double score = min_value(game_copy, -color, alpha, beta, depth - 1);
                        return make_pair(score, position.first * 8 + position.second);
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
        return bot->alpha_beta_search(game, color, depth);
    }
}

// g++ -shared -o scout_multi_thread.dll -fPIC scout_multi_thread.cpp
