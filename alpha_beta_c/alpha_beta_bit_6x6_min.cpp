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
                                        made by Microdust (Minify bit version)
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

// shift mask
#define LEFT_MASK (~LEFT_EDGE)
#define RIGHT_MASK (~RIGHT_EDGE)
#define UP_MASK (~TOP_EDGE)
#define DOWN_MASK (~BOTTOM_EDGE)
#define UP_LEFT_MASK (~(TOP_EDGE | LEFT_EDGE))
#define UP_RIGHT_MASK (~(TOP_EDGE | RIGHT_EDGE))
#define DOWN_LEFT_MASK (~(BOTTOM_EDGE | LEFT_EDGE))
#define DOWN_RIGHT_MASK (~(BOTTOM_EDGE | RIGHT_EDGE))

extern "C"
{
    const vector<int> DIRECTIONS = {LEFT, RIGHT, UP, DOWN, UP_LEFT, UP_RIGHT, DOWN_LEFT, DOWN_RIGHT};

    uint64_t shift(uint64_t board, int dir) {
        switch (dir) {
            case LEFT:          return (board & LEFT_MASK) >> 1;
            case RIGHT:         return (board & RIGHT_MASK) << 1;
            case UP:            return (board & UP_MASK) >> N;
            case DOWN:          return (board & DOWN_MASK) << N;
            case UP_LEFT:       return (board & UP_LEFT_MASK) >> (N + 1);
            case UP_RIGHT:      return (board & UP_RIGHT_MASK) >> (N - 1);
            case DOWN_LEFT:     return (board & DOWN_LEFT_MASK) << (N - 1);
            case DOWN_RIGHT:    return (board & DOWN_RIGHT_MASK) << (N + 1);
            default:            return board;
        }
    }

    void print_board(uint64_t board) {
            for (int i = 0; i < N; ++i){
                for (int j = 0; j < N; ++j)
                    if (board & POS(i, j)) cout << "1 ";
                    else cout << "0 ";
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
                if (bitboard.white & pos) cout << "O ";
                else if (bitboard.black & pos) cout << "X ";
                else cout << ". ";
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

    void getValidMoveList(Game& bitboard, int color, vector<uint64_t>& moves_list) {
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
        while (moves) {
            moves_list.push_back(moves & -moves); // 取得最低位的1
            moves &= moves - 1; // 清除最低位的1
        }
        sort(moves_list.begin(), moves_list.end(), [&](const uint64_t& a, const uint64_t& b) {
            return heuristic_score(a) > heuristic_score(b);
        });
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
                if (position & CORNER_MASK) return true;

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
                    if (stable_in_direction) return true;
                }
                return false;
            };

            for (int i = 0; i < N; ++i)
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
            if (color == WHITE && white_count <= black_count)
                return ((NN - black_count)*(NN - black_count)) / static_cast<double>(NN * NN * NN);
            if (color == BLACK && black_count <= white_count)
                return ((NN - white_count)*(NN - white_count)) / static_cast<double>(NN * NN * NN);
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
            vector<uint64_t> valids;
            getValidMoveList(bitboard, color, valids);
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
        }

        double min_value(Game bitboard, int color, double alpha, double beta, int depth)
        {
            if (isEndGame(bitboard))
                return endgame_evaluation(bitboard, -color);
            else if (depth == 0)
                return evaluate(bitboard, -color);
            vector<uint64_t> valids;
            getValidMoveList(bitboard, color, valids);
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
        }

        int alpha_beta_search(Game bitboard, int color, int depth)
        {
            double best_score = -numeric_limits<double>::infinity();
            vector<uint64_t> valids;
            getValidMoveList(bitboard, color, valids);
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
        bitboard_convert(game, bitboard);
        // double score = bot->evaluate(bitboard, color);
        // cout << "Score: " << score << endl;
        return bot->alpha_beta_search(bitboard, color, depth);
    }
}

// g++ -shared -o alpha_beta_bit_6x6_min.dll -fPIC alpha_beta_bit_6x6_min.cpp
