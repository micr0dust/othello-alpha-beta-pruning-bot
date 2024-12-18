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
#include <bitset>

using namespace::std;

#define WHITE 1
#define BLACK -1
#define CORNOR 20
#define STABLE 1
#define N 6  // 定義棋盤大小
#define NN 36
#define MAX_SCORE (STABLE * NN + CORNOR * 4 + NN)

// directions
#define LEFT 1
#define RIGHT -1
#define UP N
#define DOWN -N
#define UP_LEFT (N + 1)
#define UP_RIGHT (N - 1)
#define DOWN_LEFT (-N + 1)
#define DOWN_RIGHT (-N - 1)

// mask
#define TOP_LEFT_CORNER (1ULL << (0 * N + 0))
#define TOP_RIGHT_CORNER (1ULL << (0 * N + (N - 1)))
#define BOTTOM_LEFT_CORNER (1ULL << ((N - 1) * N + 0))
#define BOTTOM_RIGHT_CORNER (1ULL << ((N - 1) * N + (N - 1)))
#define CORNER_MASK (TOP_LEFT_CORNER | TOP_RIGHT_CORNER | BOTTOM_LEFT_CORNER | BOTTOM_RIGHT_CORNER)
#define LEFT_EDGE   0b000001000001000001000001000001000001ULL
#define RIGHT_EDGE  0b100000100000100000100000100000100000ULL
#define BOTTOM_EDGE 0b111111000000000000000000000000000000ULL
#define TOP_EDGE    0b000000000000000000000000000000111111ULL
#define POS(x, y) (1ULL << ((x) * N + (y)))

extern "C"
{
    const vector<int> DIRECTIONS = {LEFT, RIGHT, UP, DOWN, UP_LEFT, UP_RIGHT, DOWN_LEFT, DOWN_RIGHT};

    uint64_t shift(uint64_t board, int dir) {
        if (dir == LEFT) {
            return (board & ~LEFT_EDGE) >> 1;
        } else if (dir == RIGHT) {
            return (board & ~RIGHT_EDGE) << 1;
        } else if (dir == UP) {
            return (board & ~TOP_EDGE) >> N;
        } else if (dir == DOWN) {
            return (board & ~BOTTOM_EDGE) << N;
        } else if (dir == UP_LEFT) {
            return (board & ~(TOP_EDGE | LEFT_EDGE)) >> (N + 1);
        } else if (dir == UP_RIGHT) {
            return (board & ~(TOP_EDGE | RIGHT_EDGE)) >> (N - 1);
        } else if (dir == DOWN_LEFT) {
            return (board & ~(BOTTOM_EDGE | LEFT_EDGE)) << (N - 1);
        } else if (dir == DOWN_RIGHT) {
            return (board & ~(BOTTOM_EDGE | RIGHT_EDGE)) << (N + 1);
        }
        return board;
    }

    void print_board(uint64_t board) {
            for (int i = 0; i < N; ++i){
                for (int j = 0; j < N; ++j)
                    if (board & POS(i, j))
                        cout << "1 ";
                    else
                        cout << "0 ";
                cout << endl;
            }
            cout << endl;
        }

    constexpr int WEIGHTS[N][N] = {
        {100, -20, 10, 10, -20, 100},
        {-20, -50, -2, -2, -50, -20},
        { 10,  -2,  5,  5,  -2,  10},
        { 10,  -2,  5,  5,  -2,  10},
        {-20, -50, -2, -2, -50, -20},
        {100, -20, 10, 10, -20, 100}
    };

    int heuristic_score(uint64_t move) {
        int position = __builtin_ctzll(move);
        return WEIGHTS[position/N][position%N];
    }

    struct Board
    {
        int board[N][N];
    };

    struct Game
    {
        uint64_t white;
        uint64_t black;
    };

    void print_bitboard(const Game& bitboard) {
        cout << "  0 1 2 3 4 5 " << endl;
        for (int i = 0; i < N; ++i) {
            cout << i << " ";
            for (int j = 0; j < N; ++j) {
                uint64_t pos = POS(i, j);
                if (bitboard.white & pos) {
                    cout << "O ";
                } else if (bitboard.black & pos) {
                    cout << "X ";
                } else {
                    cout << ". ";
                }
            }
            cout << endl;
        }
        cout << endl;
    }

    uint64_t getValidMoves(Game& bitboard, int color) {
        uint64_t self = (color == WHITE)?bitboard.white:bitboard.black;
        uint64_t opponent = (color == WHITE)?bitboard.black:bitboard.white;
        uint64_t empty = ~(self | opponent);
        uint64_t moves=0;

        for (int dir : DIRECTIONS) {
            uint64_t potential = shift(self, dir) & opponent;
            uint64_t flip = 0;

            while (potential) {
                flip |= potential;
                potential = shift(potential, dir) & opponent;
            }

            moves |= shift(flip, dir) & empty; // update moves
        }

        return moves;
    }

    vector<uint64_t> getValidMoveList(Game& bitboard, int color) {
        uint64_t self = (color == WHITE)?bitboard.white:bitboard.black;
        uint64_t opponent = (color == WHITE)?bitboard.black:bitboard.white;
        uint64_t empty = ~(self | opponent);
        uint64_t moves = 0;

        for (int dir : DIRECTIONS) {
            uint64_t potential = shift(self, dir) & opponent;
            uint64_t flip = 0;

            while (potential) {
                flip |= potential;
                potential = shift(potential, dir) & opponent;
            }

            moves |= shift(flip, dir) & empty; // update moves
        }
        vector<uint64_t> moves_list;
        while (moves) {
            moves_list.push_back(moves & -moves); // 取得最低位的1
            moves &= moves - 1; // 清除最低位的1
        }
        sort(moves_list.begin(), moves_list.end(), [&](const uint64_t& a, const uint64_t& b) {
            return heuristic_score(a) > heuristic_score(b);
        });

        return moves_list;
    }

    void executeMove(Game& bitboard, int color, uint64_t move) {
        uint64_t self = (color == WHITE) ? bitboard.white : bitboard.black;
        uint64_t opponent = (color == WHITE) ? bitboard.black : bitboard.white;
        uint64_t flip = 0;

        for (int dir : DIRECTIONS) {
            uint64_t potential = shift(move, dir) & opponent;
            uint64_t curr_flip = 0;

            curr_flip |= potential;
            while (shift(potential, dir) & opponent) {
                potential = shift(potential, dir);
                curr_flip |= potential;
            }
            if (shift(potential, dir) & self) // can flip
                flip |= curr_flip;
        }

        // update board
        self |= move | flip; // apply move and flip
        opponent &= ~flip; // clear opponent's flipped pieces

        if (color == WHITE) {
            bitboard.white = self;
            bitboard.black = opponent;
        } else {
            bitboard.black = self;
            bitboard.white = opponent;
        }
    }

    bool isEndGame(Game& bitboard)
    {
        int white_valid_moves = __builtin_popcountll(getValidMoves(bitboard, WHITE));
        int black_valid_moves = __builtin_popcountll(getValidMoves(bitboard, BLACK));
        return white_valid_moves == 0 && black_valid_moves == 0;
    }

    class BOT
    {
    public:
        BOT() {}

        int stable_stones(Game& bitboard, int color)
        {
            uint64_t self = (color == WHITE) ? bitboard.white : bitboard.black;

            int stable_stones = 0;
            auto is_stable = [&](uint64_t position)
            {
                if (position & CORNER_MASK) {
                    return true;
                }

                for (int dir : DIRECTIONS)
                {
                    uint64_t shifted_position = position;
                    bool stable_in_direction = true;
                    while (shifted_position)
                    {
                        if (!(shifted_position & self))
                        {
                            stable_in_direction = false;
                            break;
                        }
                        shifted_position = shift(shifted_position, dir);
                    }
                    if (stable_in_direction) {
                        return true;
                    }
                }
                return false;
            };

            for (int i = 0; i < N; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    uint64_t position = POS(i, j);
                    if ((self & position) && is_stable(position))
                    {
                        stable_stones += STABLE;
                        if (position & CORNER_MASK)
                            stable_stones += CORNOR;
                    }
                }
            }
            return stable_stones;
        }

        double evaluate(Game& bitboard, int color)
        {
            int actions = __builtin_popcountll(getValidMoves(bitboard, color));
            return (actions + stable_stones(bitboard, color)) / static_cast<double>(MAX_SCORE);
        }

        double endgame_evaluation(Game& bitboard, int color)
        {
            int black_count = __builtin_popcountll(bitboard.black);
            int white_count = __builtin_popcountll(bitboard.white);
            if (color == WHITE)
                return ((NN - black_count)*(NN - black_count)) / static_cast<double>(NN * NN);
            return ((NN - white_count)*(NN - white_count)) / static_cast<double>(NN * NN);
        }

        double max_value(Game bitboard, int color, double alpha, double beta, int depth)
        {
            if (isEndGame(bitboard))
                return endgame_evaluation(bitboard, color);
            else if (depth == 0)
                return evaluate(bitboard, color);
            vector<uint64_t> valids = getValidMoveList(bitboard, color);
            if (valids.empty())
                return evaluate(bitboard, color);
            for (auto position : valids)
            {
                Game bitboard_copy = bitboard;
                executeMove(bitboard_copy, color, position);
                double score = min_value(bitboard_copy, -color, alpha, beta, depth - 1);

                alpha = max(alpha, score);
                if (beta <= alpha)
                    break;
            }
            return alpha;
            // vector<pair<int, uint64_t>> moves;
            // while (valids) {
            //     uint64_t position = valids & -valids; // 取得最低位的1
            //     valids &= valids - 1; // 清除最低位的1
            //     int pos_index = __builtin_ctzll(position); // 取得位置索引
            //     moves.emplace_back(WEIGHTS[pos_index/N][pos_index%N], position);
            // }

            // // 根據靜態表格的權重排序合法步
            // sort(moves.rbegin(), moves.rend());

            // for (const auto& move : moves) {
            //     uint64_t position = move.second;
            //     Game bitboard_copy = bitboard;
            //     executeMove(bitboard_copy, color, position);
            //     double score = min_value(bitboard_copy, -color, alpha, beta, depth - 1);

            //     alpha = max(alpha, score);
            //     if (beta <= alpha)
            //         break;
            // }
            // return alpha;
        }

        double min_value(Game bitboard, int color, double alpha, double beta, int depth)
        {
            if (isEndGame(bitboard))
                return endgame_evaluation(bitboard, -color);
            else if (depth == 0)
                return evaluate(bitboard, -color);
            vector<uint64_t> valids = getValidMoveList(bitboard, color);
            if (valids.empty())
                return evaluate(bitboard, color);
            for (auto position : valids)
            {
                Game bitboard_copy = bitboard;
                executeMove(bitboard_copy, color, position);
                double score = max_value(bitboard_copy, -color, alpha, beta, depth - 1);
                
                beta = min(beta, score);
                if (beta <= alpha)
                    break;
            }
            return beta;
            // vector<pair<int, uint64_t>> moves;
            // while (valids) {
            //     uint64_t position = valids & -valids; // 取得最低位的1
            //     valids &= valids - 1; // 清除最低位的1
            //     int pos_index = __builtin_ctzll(position); // 取得位置索引
            //     moves.emplace_back(WEIGHTS[pos_index/N][pos_index%N], position);
            // }

            // // 根據靜態表格的權重排序合法步
            // sort(moves.rbegin(), moves.rend());

            // for (const auto& move : moves) {
            //     uint64_t position = move.second;
            //     Game bitboard_copy = bitboard;
            //     executeMove(bitboard_copy, color, position);
            //     double score = min_value(bitboard_copy, -color, alpha, beta, depth - 1);

            //     beta = min(beta, score);
            //     if (beta <= alpha)
            //         break;
            // }
            // return beta;
        }

        int alpha_beta_search(Game bitboard, int color, int depth)
        {
            double best_score = -numeric_limits<double>::infinity();
            vector<uint64_t> valids = getValidMoveList(bitboard, color);
            int best_action = -1;

            vector<future<pair<double, int>>> futures;
            for (uint64_t position : valids)
            {
                futures.push_back(
                    async(launch::async, 
                    [this, bitboard, color, position, depth](){
                        Game bitboard_copy = bitboard;
                        executeMove(bitboard_copy, color, position);
                        
                        // 重新初始化 alpha 和 beta
                        double alpha = -numeric_limits<double>::infinity();
                        double beta = numeric_limits<double>::infinity();
                        
                        double score = min_value(bitboard_copy, -color, alpha, beta, depth - 1);
                        return make_pair(score, __builtin_ctzll(position));
                    }
                ));
            }

            for (auto &future : futures)
            {
                auto [score, action] = future.get();
                // cout << "action: " << action << " score: " << score << endl;
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

    void bitboard_convert(Board& game, Game& bitboard)
    {
        bitboard.black = 0;
        bitboard.white = 0;

        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                if (game.board[i][j] == BLACK)
                    bitboard.black |= POS(i, j);
                else if (game.board[i][j] == WHITE)
                    bitboard.white |= POS(i, j);
    }

    int get_action(BOT *bot, Board game, int color, int depth=5)
    {
        Game bitboard;
        bitboard.black = 0;
        bitboard.white = 0;

        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                if (game.board[i][j] == BLACK)
                    bitboard.black |= POS(i, j);
                else if (game.board[i][j] == WHITE)
                    bitboard.white |= POS(i, j);
        double score = bot->evaluate(bitboard, color);
        cout << "Score: " << score << endl;
        return bot->alpha_beta_search(bitboard, color, depth);
    }

    int stable_stonesOrigin(Board game, int color)
    {
        const vector<pair<int, int>> directions {{1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}};
        
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

    int heuristic_scoreOrigin(pair<int, int> move, const int board[N][N]) {
        return WEIGHTS[move.first][move.second];
    }

    vector<pair<int, int>> getValidMovesOrigin(Board game, int color)
    {
        const vector<pair<int, int>> directions {{1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}};
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
            return heuristic_scoreOrigin(a, game.board) > heuristic_scoreOrigin(b, game.board);
        });

        return validMoves;
    }

    double endgame_evaluationOrigin(Board game, int color)
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
            if (color == WHITE)
                return ((NN - black_count)*(NN - black_count)) / static_cast<double>(NN * NN);
            return ((NN - white_count)*(NN - white_count)) / static_cast<double>(NN * NN);
    }

    void executeMoveOrigin(Board &game, int color, pair<int, int> position)
    {
        const vector<pair<int, int>> directions {{1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}};
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

    double evaluateOrigin(Board game, int color)
    {
        int actions = getValidMovesOrigin(game, color).size();
        return (actions + stable_stonesOrigin(game, color)) / static_cast<double>(MAX_SCORE);
    }

    int unit_test_shift(BOT *bot, Board game, int color, int depth=5){
        Game bitboard;
        bitboard_convert(game, bitboard);
        cout << "UP" << endl; print_board(shift(bitboard.white, UP));
        cout << "UP_RIGHT" << endl; print_board(shift(bitboard.white, UP_RIGHT));
        cout << "RIGHT" << endl; print_board(shift(bitboard.white, RIGHT));
        cout << "DOWN_RIGHT" << endl; print_board(shift(bitboard.white, DOWN_RIGHT));
        cout << "DOWN" << endl; print_board(shift(bitboard.white, DOWN));
        cout << "DOWN_LEFT" << endl; print_board(shift(bitboard.white, DOWN_LEFT));
        cout << "LEFT" << endl; print_board(shift(bitboard.white, LEFT));
        cout << "UP_LEFT" << endl; print_board(shift(bitboard.white, UP_LEFT));
        return 0;
    }

    int unit_test_evaluate(BOT *bot, Board game, int color, int depth=5){
        Game bitboard;
        bitboard_convert(game, bitboard);
        // cout << "origin evaluate: " << evaluateOrigin(game, color) << endl;
        // cout << "bit evaluate: " << bot->evaluate(bitboard, color) << endl;
        return evaluateOrigin(game, color) == bot->evaluate(bitboard, color);
    }

    int unit_test_valid_move(BOT *bot, Board game, int color, int depth=5){
        Game bitboard;
        bitboard_convert(game, bitboard);
        uint64_t moves = getValidMoves(bitboard, color);
        // cout << "bit valid moves: " << endl;
        // print_board(moves);
        // for (int i = 0; i < NN; i++)
        //     if(moves & POS(i/N, i%N))
        //         cout << i << " ";
        // cout << endl;
        vector<pair<int, int>> list = getValidMovesOrigin(game, color);
        vector<int> actions;
        for (int i = 0; i < list.size(); i++)
            actions.push_back(list[i].first*N+list[i].second);
        // sort(actions.begin(), actions.end());
        // for(int i = 0; i < actions.size(); i++)
        //     cout << actions[i] << " ";
        // cout << endl;
        return actions.size()==__builtin_popcountll(moves);
    }

    int unit_test_execute_move(BOT *bot, Board game, int color, int depth=5)
    {
        Game bitboard;
        bitboard_convert(game, bitboard);

        uint64_t moves = getValidMoves(bitboard, color);

        while (moves){
            uint64_t position = moves & -moves; // 取得最低位的1
            moves &= moves - 1; // 清除最低位的1

            int pos_index = __builtin_ctzll(position); // 取得位置索引

            Game state_from_bit = bitboard;
            executeMove(state_from_bit, color, position); // 取得位置索引 (返回右起第一個1之後的0的個數)
            Board game_copy = game;
            executeMoveOrigin(game_copy, color, {pos_index/N,pos_index%N});
            Game state_from_array;
            bitboard_convert(game_copy, state_from_array);
            if(state_from_array.black != state_from_bit.black || state_from_array.white != state_from_bit.white){
                cout << "state_from_bit" << endl;
                print_bitboard(state_from_bit);
                cout << "state_from_array" << endl;
                print_bitboard(state_from_array);
                return 0;
            }
        }

        return 1;
    }

    int unit_test_endgame_evaluation(BOT *bot, Board game, int color, int depth=5)
    {
        Game bitboard;
        bitboard_convert(game, bitboard);

        // cout << endgame_evaluationOrigin(game, color) << endl;
        // cout << bot->endgame_evaluation(bitboard, color) << endl;

        return endgame_evaluationOrigin(game, color)==bot->endgame_evaluation(bitboard, color);
    }

    int debug(BOT *bot, Board game, int color, int depth=5)
    {
        Game bitboard;
        bitboard_convert(game, bitboard);
        uint64_t moves = getValidMoves(bitboard, color);
        print_bitboard(bitboard);

        double score = bot->evaluate(bitboard, color);
        cout << "Score: " << score << endl;
        int P = bot->alpha_beta_search(bitboard, color, depth);
        Game bitboard_copy = bitboard;
        uint64_t position = POS(P/N, P%N);
        executeMove(bitboard_copy, color, position); // 取得位置索引 (返回右起第一個1之後的0的個數)
        cout << "Move: (" << P/N << ", " << P%N << ")" << endl;
        print_bitboard(bitboard_copy);

        return P;
    }
}

// g++ -shared -o alpha_beta_bit_6x6.dll -fPIC alpha_beta_bit_6x6.cpp
