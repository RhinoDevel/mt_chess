
// Marcel Timm, RhinoDevel, 2025dec30

#ifdef __cplusplus
    #include <cassert>
    #include <cstdlib>
    #include <cstdio>
#else //__cplusplus
    #include <assert.h>
    #include <stdlib.h>
    #include <stdio.h>
#endif //__cplusplus

#include "mt_chess_str.h"
#include "mt_chess_color.h"
#include "mt_chess_piece.h"
#include "mt_chess_data.h"

#ifdef _WIN32
    #define MT_CC "\033"
#else //_WIN32
    #define MT_CC "\e"
#endif //_WIN32

#define MT_CC_COLOR_BG_BLACK (MT_CC "[40m")
#define MT_CC_COLOR_BG_RED (MT_CC "[41m")
#define MT_CC_COLOR_BG_GREEN (MT_CC "[42m")
#define MT_CC_COLOR_BG_YELLOW (MT_CC "[43m")
#define MT_CC_COLOR_BG_BLUE (MT_CC "[44m")
#define MT_CC_COLOR_BG_MAGENTA (MT_CC "[45m")
#define MT_CC_COLOR_BG_CYAN (MT_CC "[46m")
#define MT_CC_COLOR_BG_WHITE (MT_CC "[47m")
//
static char const s_cc_color_fg[] = (MT_CC "[30m");
//
static char const s_cc_color_bg[][5 + 1] = { // <- Hard-coded
    MT_CC_COLOR_BG_WHITE, // mt_chess_color_white
    MT_CC_COLOR_BG_MAGENTA // mt_chess_color_black
};
static size_t const s_cc_color_len = sizeof s_cc_color_fg;
//
static char const s_cc_reset[] = (MT_CC "[0m");
static size_t const s_cc_reset_len = sizeof s_cc_reset;

static char const unicode_piece[2][6][4] = { // <- Hard-coded
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

#define MT_BOARD_AS_STR_ASCII_ROWS 18
#define MT_BOARD_AS_STR_ASCII_COLUMNS (51 + 1) // Incl. newlines.

// Incl. 0-termination:
#define MT_BOARD_AS_STR_ASCII_CHARS \
    (MT_BOARD_AS_STR_ASCII_ROWS * MT_BOARD_AS_STR_ASCII_COLUMNS + 1)

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
    ret_val += (int)s_cc_color_len - 1;

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
        ret_val += (int)s_unicode_len - 1;
    }

    // Add second character of the square:
    dest[ret_val] = ' ';
    ++ret_val;

    return ret_val;
}

static int str_unicode_add_rank(
    struct mt_chess_data const * const data,
    int const rank,
    char * const dest)
{
    assert(data != NULL);
    assert(8 >= rank && rank >= 1);
    assert(dest != NULL);

    int ret_val = 0;

    // Rank:
    dest[ret_val] = (char)((int)'0' + rank);
    ++ret_val;

    // Space:
    dest[ret_val] = ' ';
    ++ret_val;

    // Add foreground color control code:
    snprintf(dest + ret_val, s_cc_color_len, s_cc_color_fg);
    ret_val += (int)s_cc_color_len - 1;

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
        uint8_t const piece_id = data->board[board_index];
        struct mt_chess_piece const * piece = NULL;

        if(piece_id != 0)
        {
            int const piece_index = mt_chess_piece_get_index(
                data->pieces, piece_id);

            assert(0 <= piece_index && piece_index < 2 * 2 * 8);

            piece = data->pieces + piece_index;
        }

        // Add the actual square:
        ret_val += str_unicode_add_square(square_color, piece, dest + ret_val);
    }
    
    // Add reset control code:
    snprintf(dest + ret_val, s_cc_reset_len, s_cc_reset);
    ret_val += (int)s_cc_reset_len - 1;

    // Newline:
    dest[ret_val] = '\n';
    ++ret_val;

    return ret_val;
}

static char* create_board_as_unicode(struct mt_chess_data const * const data)
{
    assert(data != NULL);

    int char_pos = 0;

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
                    + (int)s_cc_color_len - 1 // Forground color control code.
                    + 8 * ( // The squares of the row.
                        (int)s_cc_color_len - 1 // Background color ctrl. code.
                            
                            // This allows a piece on ALL squares at once, which
                            // is probably never needed (in chess):
                            +(int)s_unicode_len - 1 // As if there is a piece.

                            + 1 // Second (always empty) square character.
                        )
                    + (int)s_cc_reset_len - 1 // Reset control code per row.
                    + 1 // Trailing newline per row.
            )
            + (18 + 1) // Bottom row (showing a, b, c, d, e, f, g and h).
            + 1; // Trailing zero-terminator.
    char * const ret_val = (char*)malloc((size_t)max_chars * sizeof *ret_val);

    assert(ret_val != NULL);

    // Ranks (rows of the actual board):
    for(int rank = 8; 1 <= rank; --rank)
    {
        char_pos += str_unicode_add_rank(data, rank, ret_val + char_pos);
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

static char* create_board_as_ascii(struct mt_chess_data const * const data)
{
    assert(data != NULL);
                                  
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

    char * const ret_val = (char*)malloc(
        MT_BOARD_AS_STR_ASCII_CHARS * sizeof *ret_val);
    
    assert(ret_val != NULL);
    
    for(int row = 0; row < MT_BOARD_AS_STR_ASCII_ROWS; ++row)
    {
        int const row_offset = row * MT_BOARD_AS_STR_ASCII_COLUMNS;
        bool const is_sep = row % 2 == 0;
        
        for(int col = 0; col < MT_BOARD_AS_STR_ASCII_COLUMNS; ++col)
        {
            int const col_offset = row_offset + col;
            
            if(is_sep)
            {
                if(col < 2)
                {
                    ret_val[col_offset] = ' ';
                    continue;
                }
                if(col + 1 < MT_BOARD_AS_STR_ASCII_COLUMNS)
                {
                    ret_val[col_offset] = '-';
                    continue;
                }
                ret_val[col_offset] = '\n';
                continue;
            }
            
            if(row + 1 == MT_BOARD_AS_STR_ASCII_ROWS)
            {
                int const buf = col - 5;
                
                if(buf % 6 == 0)
                {
                    ret_val[col_offset] = (char)((int)'a' + buf / 6);
                    continue;
                }
                if(col + 1 < MT_BOARD_AS_STR_ASCII_COLUMNS)
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
            
            if(3 <= col && col < MT_BOARD_AS_STR_ASCII_COLUMNS - 2)
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
                    
                    uint8_t const piece_id = data->board[board_index];
                    
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
                                data->pieces, piece_id);
                        
                        assert(0 <= piece_index && piece_index < 2 * 2 * 8);
                        
                        if(data->pieces[piece_index].color
                            == mt_chess_color_black)
                        {
                            ret_val[col_offset + 1] = 'b'; // Black piece.
                        }
                        else
                        {
                            assert(data->pieces[piece_index].color
                                == mt_chess_color_white);
                            
                            ret_val[col_offset + 1] = 'w'; // White piece.
                        }
                        
                        ret_val[col_offset + 2] = '_';
                        
                        switch(data->pieces[piece_index].type)
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
            
            if(col == MT_BOARD_AS_STR_ASCII_COLUMNS - 2)
            {
                ret_val[col_offset] = '|';
                continue;
            }
            ret_val[col_offset] = '\n';
        }
    }
    
    ret_val[MT_BOARD_AS_STR_ASCII_CHARS - 1] = '\0';
    
    return ret_val;
}

char* mt_chess_str_create_board(
    struct mt_chess_data const * const data, bool const unicode)
{
    if(unicode)
    {
        return create_board_as_unicode(data);
    }
    return create_board_as_ascii(data);
}
