
// Marcel Timm, RhinoDevel, 2025dec21

#ifdef __cplusplus
    #include <cstdint>
    #include <cassert>
    #include <cstdbool>
    #include <cstdlib>
    #include <cstdio>
#else //__cplusplus
    #include <stdint.h>
    #include <assert.h>
    #include <stdbool.h>
    #include <stdlib.h>
    #include <stdio.h>
#endif //__cplusplus

#include "mt_chess_data.h"
#include "mt_chess_piece.h"
#include "mt_chess_color.h"
#include "mt_chess_type.h"
#include "mt_chess_col.h"
#include "mt_chess.h"

// TODO: These control codes are NOT compatible with (e.g.) Windows:
//
static char const s_cc_color_fg[] =
    "\e[30m";
static char const s_cc_color_bg[][5 + 1] = {
    "\e[47m", // mt_chess_color_white
    "\e[45m" // mt_chess_color_black
};
static size_t const s_cc_color_len = sizeof s_cc_color_fg;

static char const s_cc_reset[] = "\e[0m";
static size_t const s_cc_reset_len = sizeof s_cc_reset;

static char const unicode_piece[2][6][4] = {
    { // mt_chess_color_white
        "\u2654", // mt_chess_type_king
        "\u2659", // mt_chess_type_pawn
        "\u2658", // mt_chess_type_knight
        "\u2657", // mt_chess_type_bishop
        "\u2656", // mt_chess_type_rook
        "\u2655" // mt_chess_type_queen
    },
    { // mt_chess_color_black
        "\u265A", // mt_chess_type_king
        "\u265F", // mt_chess_type_pawn
        "\u265E", // mt_chess_type_knight
        "\u265D", // mt_chess_type_bishop
        "\u265C", // mt_chess_type_rook
        "\u265B" // mt_chess_type_queen
    }
};
static size_t const s_unicode_len = sizeof unicode_piece[0][0];

static struct mt_chess_data * s_data = NULL;

static bool is_move_allowed(
    struct mt_chess_piece const * const piece,
    struct mt_chess_pos const * const from,
    struct mt_chess_pos const * const to,
    char const * * const out_msg)
{
    assert(piece != NULL && piece->id != 0);
    assert(from != NULL && !mt_chess_pos_is_invalid(from));
    assert(to != NULL && !mt_chess_pos_is_invalid(to));
    assert(out_msg != NULL);
    
    *out_msg = NULL;
    
    if(piece->color != s_data->turn)
    {
        *out_msg = "It is the other player's turn.";
        return false;
    }

    if(mt_chess_pos_are_equal(from, to))
    {
        //assert(false); // The UI should prevent getting here?
        *out_msg = "The given from- and to-positions are equal.";
        return false;
    }
    
    int const to_board_index = ((int)mt_chess_col_h + 1) * to->row + to->col;
    assert(0 <= to_board_index && to_board_index < 8 * 8);


    int const to_piece_id = s_data->board[to_board_index];
    if(to_piece_id != 0)
    {
        // There is a(-nother) piece on the destination square.

        int const to_piece_index = mt_chess_piece_get_index(
                s_data->pieces, to_piece_id);
        struct mt_chess_piece const * const to_piece =
            s_data->pieces + to_piece_index;

        if(to_piece->color == s_data->turn)
        {
            *out_msg = "There is another piece belonging to the current player on the destination square.";
            return false;
        }

        // Opponent's piece on the destination square.

        // Must have been avoided earlier on.
        assert(to_piece->type != mt_chess_type_king);
    }
    
    assert(*out_msg == NULL);
    return true;
}

/**
 * - Does not set the foreground color.
 */
static int str_unicode_add_square(
    enum mt_chess_color const square_color,
    struct mt_chess_piece const * const piece, // May be NULL.
    char * const dest)
{
    assert(
        square_color == mt_chess_color_white
            || square_color == mt_chess_color_black);
    assert(piece == NULL || piece->id != 0);
    assert(dest != NULL);

    int ret_val = 0;

    // Add control code to set the square's color:
    snprintf(dest + ret_val, s_cc_color_len, s_cc_color_bg[(int)square_color]);
    ret_val += s_cc_color_len - 1;

    // Add first character of square:
    if(piece == NULL)
    {
        dest[ret_val] = ' ';
        ++ret_val;
    }
    else
    {
        snprintf(
            dest + ret_val,
            s_unicode_len,
            unicode_piece[piece->color][piece->type]);
        ret_val += s_unicode_len - 1;
    }

    // Add second character of the square:
    dest[ret_val] = ' ';
    ++ret_val;

    return ret_val;
}

static int str_unicode_add_rank(int const rank, char * const dest)
{
    assert(8 >= rank && rank >= 1);
    assert(dest != NULL);
    assert(s_data != NULL);

    int ret_val = 0;

    // Rank:
    dest[ret_val] = (char)((int)'0' + rank);
    ++ret_val;

    // Space:
    dest[ret_val] = ' ';
    ++ret_val;

    // Add foreground color control code:
    snprintf(dest + ret_val, s_cc_color_len, s_cc_color_fg);
    ret_val += s_cc_color_len - 1;

    enum mt_chess_color const first_square_color =
            rank % 2 == 0 ? mt_chess_color_white : mt_chess_color_black;

    assert(1 - mt_chess_color_white == mt_chess_color_black);
    assert(mt_chess_color_white == 1 - mt_chess_color_black);

    int const row = 7 - rank + 1;
    int const row_offset = row * ((int)mt_chess_col_h + 1);

    for(int col = 0; col < 8; ++col)
    {
        enum mt_chess_color const square_color =
            col % 2 == 0 ? first_square_color : (1 - first_square_color);

        int const board_index = row_offset + col;
        assert(board_index < 8 * 8);
        int const piece_id = s_data->board[board_index];
        int piece_index = -1;
        struct mt_chess_piece const * piece = NULL;

        if(piece_id != 0)
        {
            int const piece_index = mt_chess_piece_get_index(
                s_data->pieces, piece_id);

            assert(0 <= piece_index && piece_index < 2 * 2 * 8);

            piece = s_data->pieces + piece_index;
        }

        // Add the actual square:
        ret_val += str_unicode_add_square(square_color, piece, dest + ret_val);
    }
    
    // Add reset control code:
    snprintf(dest + ret_val, s_cc_reset_len, s_cc_reset);
    ret_val += s_cc_reset_len - 1;

    // Newline:
    dest[ret_val] = '\n';
    ++ret_val;

    return ret_val;
}

static char* create_board_as_str_unicode(void)
{
    int char_pos = 0;

    if(s_data == NULL)
    {
        // Not initialized, yet.
        assert(false);
        return NULL;
    }

    // 000000000011111111 1 1
    // 012345678901234567 8 9
    // 8 __##__##__##__##\n
    // 7 ##__##__##__##__\n
    // 6 __##__##__##__##\n
    // 5 ##__##__##__##__\n
    // 4 __##__##__##__##\n
    // 3 ##__##__##__##__\n
    // 2 __##__##__##__##\n
    // 1 ##__##__##__##__\n
    //   a b c d e f g h \n\0
    int const max_chars = // Hard-coded [e.g. see str_unicode_add_square()].
        8 * ( // Rows (ranks) of the board (8, 7, 6, 5, 4, 3, 2 and 1).
                1 + 1 // Row number and space.
                    + s_cc_color_len - 1 // Forground color control code.
                    + 8 * ( // The squares of the row.
                            s_cc_color_len - 1 // Background color control code.
                            
                            // This allows a piece on ALL squares at once, which
                            // is probably never needed (in chess):
                            + s_unicode_len - 1 // As if there is a piece.

                            + 1 // Second (always empty) square character.
                        )
                    + s_cc_reset_len - 1 // Reset control code per row.
                    + 1 // Trailing newline per row.
            )
            + (18 + 1) // Bottom row (showing a, b, c, d, e, f, g and h).
            + 1; // Trailing zero-terminator.
    char * const ret_val = (char*)malloc((size_t)max_chars * sizeof *ret_val);

    assert(ret_val != NULL);

    // Ranks (rows of the actual board):
    for(int rank = 8; 1 <= rank; --rank)
    {
        char_pos += str_unicode_add_rank(rank, ret_val + char_pos);
    }

    // Last row (with file "titles"):
    ret_val[char_pos] = ' ';
    ++char_pos;
    ret_val[char_pos] = ' ';
    ++char_pos;
    for(int i = 0; i < 8; ++i)
    {
        ret_val[char_pos] = (char)((int)'a' + i);
        ++char_pos;
        ret_val[char_pos] = ' ';
        ++char_pos;
    }
    ret_val[char_pos] = '\n';
    ++char_pos;

    // Null-termination:
    ret_val[char_pos] = '\0';
    ++char_pos;

    assert(char_pos <= max_chars);
    return ret_val;
}

static char* create_board_as_str_ascii(void)
{
    if(s_data == NULL)
    {
        // Not initialized, yet.
        assert(false);
        return NULL;
    }
                                  
    // 0000000000111111111122222222223333333333444444444455
    // 0123456789012345678901234567890123456789012345678901
    //   -------------------------------------------------\n    0
    // 8 | b_r | b_n | b_b | b_q | b_k | b_b | b_n | b_r |\n    1  1/2 = 0; 8 - 0 = 8
    //   -------------------------------------------------\n    2 
    // 7 | b_p | b_p | b_p | b_p | b_p | b_p | b_p | b_p |\n    3  3/2 = 1; 8 - 1 = 7
    //   -------------------------------------------------\n    4
    // 6 |#####|     |     |#####|     |     |#####|     |\n    5  5/2 = 2; 8 - 2 = 6
    //   -------------------------------------------------\n    6
    // 5 |     |#####|     |#####|     |#####|     |#####|\n    7  7/2 = 3; 8 - 3 = 5
    //   -------------------------------------------------\n    8
    // 4 |#####|     |     |#####|     |     |#####|     |\n    9  9/2 = 4; 8 - 4 = 4
    //   -------------------------------------------------\n   10
    // 3 |     |#####|     |#####|     |#####|     |#####|\n   11 11/2 = 5; 8 - 5 = 3
    //   -------------------------------------------------\n   12
    // 2 | w_p | w_p | w_p | w_p | w_p | w_p | w_p | w_p |\n   13 13/2 = 6; 8 - 6 = 2
    //   ------------------------------------------------|\n   14 
    // 1 | w_r | w_n | w_b | w_q | w_k | w_b | w_n | w_r |\n   15 15/2 = 7; 8 - 7 = 1
    //   ------------------------------------------------|\n   16
    //      a     b     c     d     e     f     g     h   \n\0 17
    
    static int const rows = 18;
    static int const columns = 51 + 1; // Incl. newlines.
    static int const chars = rows * columns + 1; // Incl. 0-termination.
    char * const ret_val = (char*)malloc((size_t)chars * sizeof *ret_val);
    
    assert(ret_val != NULL);
    
    for(int row = 0; row < rows; ++row)
    {
        int const row_offset = row * columns;
        bool const is_sep = row % 2 == 0;
        
        for(int col = 0; col < columns; ++col)
        {
            int const col_offset = row_offset + col;
            
            if(is_sep)
            {
                if(col < 2)
                {
                    ret_val[col_offset] = ' ';
                    continue;
                }
                if(col + 1 < columns)
                {
                    ret_val[col_offset] = '-';
                    continue;
                }
                ret_val[col_offset] = '\n';
                continue;
            }
            
            if(row + 1 == rows)
            {
                int const buf = col - 5;
                
                if(buf % 6 == 0)
                {
                    ret_val[col_offset] = (char)((int)'a' + buf / 6);
                    continue;
                }
                if(col + 1 < columns)
                {
                    ret_val[col_offset] = ' ';
                    continue;
                }
                ret_val[col_offset] = '\n';
                continue;
            }
            
            if(col == 0)
            {
                int const rank = 8 - row / 2;
                
                if(0 < rank)
                {
                    ret_val[col_offset] = (char)((int)'0' + rank);
                    continue;
                }
                ret_val[col_offset] = ' ';
                continue;
            }
            if(col == 1)
            {
                ret_val[col_offset] = ' ';
                continue;
            }
            if(col == 2)
            {
                ret_val[col_offset] = '|';
                continue;
            }
            
            if(3 <= col && col < columns - 2)
            {
                if((col - 2) % 6 == 0)
                {
                    ret_val[col_offset] = '|';
                    continue;
                }
            
                int const buf = col - 3;
                
                if(buf % 6 == 0)
                {
                    int const board_col = buf / 6;
                    int const board_row = row / 2;
                    int const board_index =
                            board_row * ((int)mt_chess_col_h + 1) + board_col;
                            
                    assert(board_index < 8 * 8);
                    
                    uint8_t const piece_id = s_data->board[board_index];
                    
                    if(piece_id == 0)
                    {
                        // Empty.
                        
                        if(board_row % 2 == board_col % 2)
                        {
                            // White cell.
                            
                            ret_val[col_offset] = '#';
                            ret_val[col_offset + 1] = '#';
                            ret_val[col_offset + 2] = '#';
                            ret_val[col_offset + 3] = '#';
                            ret_val[col_offset + 4] = '#';
                        }
                        else
                        {
                            // Black cell.
                            
                            ret_val[col_offset] = ' ';
                            ret_val[col_offset + 1] = ' ';
                            ret_val[col_offset + 2] = ' ';
                            ret_val[col_offset + 3] = ' ';
                            ret_val[col_offset + 4] = ' ';
                        }
                    }
                    else
                    {
                        // NOT empty.
                        
                        ret_val[col_offset] = ' ';
                        
                        int const piece_index = mt_chess_piece_get_index(
                                s_data->pieces, piece_id);
                        
                        assert(0 <= piece_index && piece_index < 2 * 2 * 8);
                        
                        if(s_data->pieces[piece_index].color
                            == mt_chess_color_black)
                        {
                            ret_val[col_offset + 1] = 'b'; // Black piece.
                        }
                        else
                        {
                            assert(s_data->pieces[piece_index].color
                                == mt_chess_color_white);
                            
                            ret_val[col_offset + 1] = 'w'; // White piece.
                        }
                        
                        ret_val[col_offset + 2] = '_';
                        
                        switch(s_data->pieces[piece_index].type)
                        {
                            case mt_chess_type_king:
                            {
                                ret_val[col_offset + 3] = 'k'; // King
                                break;
                            }
                            case mt_chess_type_pawn:
                            {
                                ret_val[col_offset + 3] = 'p'; // Pawn
                                break;
                            }
                            case mt_chess_type_knight:
                            {
                                ret_val[col_offset + 3] = 'n'; // Knight
                                break;
                            }
                            case mt_chess_type_bishop:
                            {
                                ret_val[col_offset + 3] = 'b'; // Bishop
                                break;
                            }
                            case mt_chess_type_rook:
                            {
                                ret_val[col_offset + 3] = 'r'; // Rook
                                break;
                            }
                            case mt_chess_type_queen:
                            {
                                ret_val[col_offset + 3] = 'q'; // Queen
                                break;
                            }
                            
                            default:
                            {
                                assert(false);
                                ret_val[col_offset + 3] = '?'; // Error!
                                break;
                            }
                        }
                        
                        ret_val[col_offset + 4] = ' ';
                    }
                    col += 4; // Kind of a hack..
                    continue;
                }
                
                ret_val[col_offset] = '?';
                continue;
            }
            
            if(col == columns - 2)
            {
                ret_val[col_offset] = '|';
                continue;
            }
            ret_val[col_offset] = '\n';
        }
    }
    
    ret_val[chars - 1] = '\0';
    
    return ret_val;
}

MT_EXPORT_CHESS_API void __stdcall mt_chess_free(void * const ptr)
{
    if(ptr == NULL)
    {
        assert(false); // Although no problem, here.
        return;
    }
    free(ptr);
}

MT_EXPORT_CHESS_API void __stdcall mt_chess_deinit(void)
{
    if(s_data == NULL)
    {
        assert(false); // Although no problem.
        return;
    }
    mt_chess_data_free(s_data);
    s_data = NULL;
}

MT_EXPORT_CHESS_API void __stdcall mt_chess_reinit(void)
{
    if(s_data != NULL)
    {
        mt_chess_deinit();
    }
    assert(s_data == NULL);
    s_data = mt_chess_data_create();
    assert(s_data != NULL);
}

MT_EXPORT_CHESS_API char* __stdcall mt_chess_create_board_as_str(
    bool const unicode)
{
    if(unicode)
    {
        return create_board_as_str_unicode();
    }
    return create_board_as_str_ascii();
}

MT_EXPORT_CHESS_API bool __stdcall mt_chess_try_move(
    char const from_file, char const from_rank,
    char const to_file, char const to_rank,
    char const * * const out_msg)
{
    assert(out_msg != NULL);
    *out_msg = NULL;
    
    struct mt_chess_pos const from = mt_chess_pos_get(from_file, from_rank);
    
    if(mt_chess_pos_is_invalid(&from))
    {
        assert(false); // Should have already been handled by caller.
        *out_msg = "The given from-position is invalid.";
        return false;
    }
    
    struct mt_chess_pos const to = mt_chess_pos_get(to_file, to_rank);
    
    if(mt_chess_pos_is_invalid(&to))
    {
        assert(false); // Should have already been handled by caller.
        *out_msg = "The given to-position is invalid.";
        return false;
    }
    
    int const piece_board_index =
            ((int)mt_chess_col_h + 1) * from.row + from.col;
    assert(0 <= piece_board_index && piece_board_index < 8 * 8);
    uint8_t const piece_id = s_data->board[piece_board_index];
    
    if(piece_id == 0)
    {
        *out_msg = "There is no piece at from-position.";
        return false;
    }

    int const piece_index = mt_chess_piece_get_index(s_data->pieces, piece_id);
    
    assert(0 <= piece_index);
    
    struct mt_chess_piece const * const piece = s_data->pieces + piece_index;
    assert(piece->id == piece_id);
    
    if(!is_move_allowed(piece, &from, &to, out_msg))
    {
        assert(*out_msg != NULL);
        return false;
    }
    
    // TODO: Actually move both pieces during castling.
    // TODO: Actually remove other piece "on passant".
    
    s_data->board[piece_board_index] = 0;
    
    int const to_board_index = ((int)mt_chess_col_h + 1) * to.row + to.col;
    assert(0 <= to_board_index && to_board_index < 8 * 8);
    
    s_data->board[to_board_index] = piece->id;
    
    // TODO: Implement logging!
    
    s_data->turn = (enum mt_chess_color)(1 - (int)s_data->turn);

    assert(*out_msg == NULL);
    return true;
}