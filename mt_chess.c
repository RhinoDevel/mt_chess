
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
#include "mt_chess_log_node.h"

// TODO: These control codes are NOT compatible with (e.g.) Windows:
//
#define MT_CC_COLOR_BG_BLACK "\e[40m"
#define MT_CC_COLOR_BG_RED "\e[41m"
#define MT_CC_COLOR_BG_GREEN "\e[42m"
#define MT_CC_COLOR_BG_YELLOW "\e[43m"
#define MT_CC_COLOR_BG_BLUE "\e[44m"
#define MT_CC_COLOR_BG_MAGENTA "\e[45m"
#define MT_CC_COLOR_BG_CYAN "\e[46m"
#define MT_CC_COLOR_BG_WHITE "\e[47m"
//
static char const s_cc_color_fg[] =
    "\e[30m";
static char const s_cc_color_bg[][5 + 1] = {
    "\e[47m", // mt_chess_color_white
    MT_CC_COLOR_BG_MAGENTA // mt_chess_color_black
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

static bool is_move_allowed_king(
    struct mt_chess_pos const * const from,
    struct mt_chess_pos const * const to,
    char const * * const out_msg)
{
    assert(from != NULL && !mt_chess_pos_is_invalid(from));
    assert(to != NULL && !mt_chess_pos_is_invalid(to));
    assert(out_msg != NULL);

    assert(!mt_chess_pos_are_equal(from, to)); // Must have been checked before.
    assert(*out_msg == NULL);

    // 0| | | |
    // --------
    // 1| |k| |
    // --------
    // 2| | | |
    // --------
    //  |0|1|2|

    // Is the to-square vertically equal or adjacent to the from-square?

    int const vert_dist = abs((int)to->row - (int)from->row);

    if(1 < vert_dist)
    {
        *out_msg = "A king can move at most one rank per turn.";
        return false;
    }

    int const horiz_dist = abs((int)to->col - (int)from->col);

    if(2 < horiz_dist)
    {
        // Neither a "normal", nor a castling move.
        *out_msg = "A king can move at most two files per turn.";
        return false;
    }

    if(horiz_dist == 2)
    {
        // Could be castling.

        // TODO:
        // - Is the to-square one of the valid castling destinations?
        // - Is it the king's first move?
        // - Did the rook never move?
        // - Are there no pieces in the way (of either king or rook)?
        // - Is the king not attacked on the from-square?
        // - Does the king not move through an attacked field?
        // - Is to-square not attacked?
        //
        assert(false); // Not implemented, yet!
        *out_msg = "CASTLING-CHECK IS NOT IMPLEMENTED, YET!";
        return false;
    }

    // NOT castling.

    assert(*out_msg == NULL);
    return true;
}

static bool is_move_allowed_knight(
    struct mt_chess_pos const * const from,
    struct mt_chess_pos const * const to,
    char const * * const out_msg)
{
    assert(from != NULL && !mt_chess_pos_is_invalid(from));
    assert(to != NULL && !mt_chess_pos_is_invalid(to));
    assert(out_msg != NULL);

    assert(*out_msg == NULL);

    // 0| |x| |x| |
    // ------------
    // 1|x| | | |x|
    // ------------
    // 2| | |n| | |
    // ------------
    // 3|x| | | |x|
    // ------------
    // 4| |x| |x| |
    // ------------
    //  |0|1|2|3|4|

    int const vert_dist = abs((int)to->row - (int)from->row);
    int const horiz_dist = abs((int)to->col - (int)from->col);

    if((vert_dist == 2 && horiz_dist == 1)
        || (vert_dist == 1 && horiz_dist == 2))
    {
        assert(*out_msg == NULL);
        return true; // Seems to be a OK move.
    }
    *out_msg = "A knight cannot move this way.";
    return false;
}

static bool is_move_allowed_bishop(
    struct mt_chess_pos const * const from,
    struct mt_chess_pos const * const to,
    char const * * const out_msg)
{
    assert(from != NULL && !mt_chess_pos_is_invalid(from));
    assert(to != NULL && !mt_chess_pos_is_invalid(to));
    assert(out_msg != NULL);

    assert(*out_msg == NULL);

    uint8_t col = 0;
    uint8_t last_col = 0;
    
    uint8_t row = 0;
    uint8_t last_row = 0;

    if(from->col < to->col)
    {
        col = from->col + 1;
        last_col = to->col - 1;
    }
    else
    {
        assert(to->col <= from->col);
        col = to->col + 1;
        last_col = from->col - 1;
    }

    if(from->row < to->row)
    {
        row = from->row + 1;
        last_row = to->row - 1;
    }
    else
    {
        assert(to->row <= from->row);
        row = to->row + 1;
        last_row = from->row - 1;
    }

    if(last_row - row != last_col - col)
    {
        *out_msg = "A bishop can move diagonally, only.";
        return false;
    }

    while(row <= last_row)
    {
        assert(col <= last_col);

        int const row_offset = ((int)mt_chess_col_h + 1) * row;
        int const board_index = row_offset + col;
        assert(0 <= board_index && board_index < 8 * 8);

        if(s_data->board[board_index] != 0)
        {
            *out_msg = "There is at least one piece in the bishop's path.";
            return false;
        }

        ++row;
        ++col;
    }
    assert(last_col + 1 == col);

    assert(*out_msg == NULL);
    return true;
}

static bool is_move_allowed_rook(
    struct mt_chess_pos const * const from,
    struct mt_chess_pos const * const to,
    char const * * const out_msg)
{
    assert(from != NULL && !mt_chess_pos_is_invalid(from));
    assert(to != NULL && !mt_chess_pos_is_invalid(to));
    assert(out_msg != NULL);

    assert(*out_msg == NULL);

    if(from->row == to->row)
    {
        assert(from->col != to->col); // Must have been checked before.

        // A horizontal move.

        int board_index = 0;
        int last_board_index = 0;

        if(from->col < to->col)
        {
            board_index = from->col + 1;
            last_board_index = to->col - 1;
        }
        else
        {
            assert(to->col <= from->col);
            board_index = to->col + 1;
            last_board_index = from->col - 1;
        }

        int const row_offset =
                ((int)mt_chess_col_h + 1) * from/*to*/->row;

        board_index += row_offset;
        assert(0 <= board_index && board_index < 8 * 8);
        last_board_index += row_offset;
        assert(0 <= last_board_index && last_board_index < 8 * 8);

        while(board_index <= last_board_index)
        {
            if(s_data->board[board_index] != 0)
            {
                *out_msg = "There is at least one piece blocking the rook's path on its rank.";
                return false;
            }
            ++board_index;
        }

        assert(*out_msg == NULL);
        return true; // Seems to be an OK move.
    }

    if(from->col != to->col)
    {
        // Suggested horizontal & vertical move. => Not supported.
        *out_msg = "A rook can either move on a rank or a file, not both.";
        return false;
    }

    // A vertical move.

    uint8_t row = 0;
    uint8_t last_row = 0;

    if(from->row < to->row)
    {
        row = from->row + 1;
        last_row = to->row - 1;
    }
    else
    {
        assert(to->row <= from->row);
        row = to->row + 1;
        last_row = from->row - 1;
    }

    while(row <= last_row)
    {
        int const row_offset = ((int)mt_chess_col_h + 1) * row;
        int const board_index = row_offset + from/*to*/->col;

        assert(0 <= board_index && board_index < 8 * 8);

        if(s_data->board[board_index] != 0)
        {
            *out_msg = "There is at least one piece blocking the rook's path on its file.";
            return false;
        }

        ++row;
    }
    assert(*out_msg == NULL);
    return true; // Seems to be an OK move.
}

static bool is_move_allowed_queen(
    struct mt_chess_pos const * const from,
    struct mt_chess_pos const * const to,
    char const * * const out_msg)
{
    assert(from != NULL && !mt_chess_pos_is_invalid(from));
    assert(to != NULL && !mt_chess_pos_is_invalid(to));
    assert(out_msg != NULL);

    assert(*out_msg == NULL);

    if(is_move_allowed_rook(from, to, out_msg))
    {
        assert(*out_msg == NULL);
        return true; // Seems to be an OK move.
    }
    assert(*out_msg != NULL);
    *out_msg = NULL; // To avoid assertion in bishop function..
    if(is_move_allowed_bishop(from, to, out_msg))
    {
        assert(*out_msg == NULL);
        return true; // Seems to be an OK move.
    }
    assert(*out_msg != NULL); // Although msg. will be overwritten.
    // Unfortun. a vague msg. to be able to use the rook & bishop functions..
    *out_msg = "Queen cannot move this way or is blocked by a piece in the way.";
    return false;
}

static bool is_move_allowed_pawn(
    struct mt_chess_piece const * const piece,
    struct mt_chess_pos const * const from,
    struct mt_chess_pos const * const to,
    uint8_t const to_piece_id,
    uint8_t * const out_remove_piece_id,
    char const * * const out_msg)
{
    assert(piece != NULL && piece->id != 0);
    assert(from != NULL && !mt_chess_pos_is_invalid(from));
    assert(to != NULL && !mt_chess_pos_is_invalid(to));
    assert(out_remove_piece_id != NULL);
    assert(out_msg != NULL);

    assert(*out_msg == NULL);
    assert(*out_remove_piece_id == 0);

    // White has negative direction, because of rank order (8 to 1).
    int const vert_dir = piece->color == mt_chess_color_white ? -1 : 1;
    int const vert_dist = vert_dir * (to->row - from->row);

    int const horiz_dist = abs((int)to->col - (int)from->col);

    if(vert_dist == 0)
    {
        // (must have already been verified that the positions are not equal)
        assert(0 < horiz_dist);

        *out_msg = "Pawns cannot move to the sides.";
        return false;
    }

    // Pawn moves vertically.

    if(vert_dist < 0)
    {
        *out_msg = "Pawns cannot move backwards.";
        return false;
    }

    // Pawn moves forward.

    if(2 < vert_dist)
    {
        *out_msg = "Pawns can move at most two squares forward.";
        return false;
    }

    // Pawn moves 1 or 2 squares forward.

    if(vert_dist != 1)
    {
        // Pawn moves 2 squares forward.

        assert(vert_dist == 2); // Checked above.

        if(horiz_dist != 0)
        {
            *out_msg = "Pawns cannot move two squares forward while also moving to the side.";
            return false;
        }

        // Pawn moves 2 squares forward straight.

        enum mt_chess_row const first_row =
            piece->color == mt_chess_color_white
                ? mt_chess_row_2 : mt_chess_row_7;

        if(from->row != first_row)
        {
            *out_msg = "Pawns can move two forward squares at once for their first move, only.";
            return false;
        }

        // Pawn moves 2 squares forward straight for their first move.

        // Should work for black and white:
        int const middle_row = (from->row + to->row) / 2;
        int const middle_board_index = middle_row * (mt_chess_row_1 + 1)
                + from/*to*/->col;

        if(s_data->board[middle_board_index] != 0)
        {
            *out_msg = "A pawn cannot move two squares in straight forward direction, if there is another piece in-between.";
            return false;
        }

        if(to_piece_id != 0)
        {
            *out_msg = "A pawn cannot catch while moving two squares in straight forward direction.";
            return false;
        }

        assert(*out_remove_piece_id == 0);
        return true; // Seems to be an OK move.
    }

    // Pawn moves 1 square forward.

    if(1 < horiz_dist)
    {
        *out_msg = "A pawn can move at most one square forward and to the side at once.";
        return false;
    }

    // Pawn moves 1 square forward and at most 1 square to a side.

    if(horiz_dist == 1)
    {
        // Pawn moves 1 square forward and 1 square to the side.
        if(to_piece_id != 0)
        {
            // There is an opponent's piece at the destination square.
            return true; // Seems to be an OK move.  
        }
        // There is no (opponent's) piece at the destination square.

        struct mt_chess_log_node const * const latest =
            mt_chess_log_node_get_latest(s_data->log);

        if(latest == NULL)
        {
            *out_msg = "A pawn cannot move one square diagonally as initial move.";
            return false;
        }
        // There was at least one preceding move.
        assert(latest->move.piece.color != s_data->turn);
        if(latest->move.piece.type != mt_chess_type_pawn)
        {
            *out_msg = "A pawn can at most move one square diagonally after an opponent's pawn's move.";
            return false;
        }
        // The last move was a pawn's move.

        if(to->col == latest->move.to/*from*/.col
            && ( // Turn-color must be white:
                (from->row == mt_chess_row_5
                && to->row == mt_chess_row_6
                && latest->move.from.row == mt_chess_row_7
                && latest->move.to.row == mt_chess_row_5)
                // Turn color must be black:
                || (from->row == mt_chess_row_4
                    && to->row == mt_chess_row_3
                    && latest->move.from.row == mt_chess_row_2
                    && latest->move.to.row == mt_chess_row_4)))
        {
            assert(to_piece_id == 0); // There must never be a piece there.

            // "En passant" detected.
            *out_remove_piece_id = latest->move.piece.id;
            return true; // Seems to be an OK move. 
        }

        *out_msg = "Not an \"en passant\" move.";
        return false;
    }

    // Pawn moves 1 square forward straight.

    assert(horiz_dist == 0);

    if(to_piece_id != 0)
    {
        *out_msg = "A pawn cannot catch while moving one square in straight forward direction.";
        return false;
    }

    assert(*out_remove_piece_id == 0);
    return true; // Seems to be an OK move.
}

static bool is_move_allowed(
    struct mt_chess_piece const * const piece,
    struct mt_chess_pos const * const from,
    struct mt_chess_pos const * const to,
    uint8_t * const out_remove_piece_id,
    char const * * const out_msg)
{
    assert(piece != NULL && piece->id != 0);
    assert(from != NULL && !mt_chess_pos_is_invalid(from));
    assert(to != NULL && !mt_chess_pos_is_invalid(to));
    assert(out_remove_piece_id != NULL);
    assert(out_msg != NULL);
    
    struct mt_chess_piece const * to_piece = NULL;

    *out_remove_piece_id = 0;
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

    uint8_t const to_piece_id = s_data->board[to_board_index];

    if(to_piece_id != 0)
    {
        // There is a(-nother) piece on the destination square.

        int const to_piece_index = mt_chess_piece_get_index(
                s_data->pieces, to_piece_id);
        
        to_piece = s_data->pieces + to_piece_index;
        if(to_piece->color == s_data->turn)
        {
            *out_msg = "There is another piece belonging to the current player on the destination square.";
            return false;
        }

        // Opponent's piece on the destination square.

        // Must have been avoided earlier on.
        assert(to_piece->type != mt_chess_type_king);
    }
    
    switch(piece->type)
    {
        case mt_chess_type_king:
        {
            if(!is_move_allowed_king(from, to, out_msg))
            {
                assert(*out_msg != NULL);
                assert(*out_remove_piece_id == 0); // Although does not matter.
                return false;
            }
            break;
        }
        case mt_chess_type_pawn:
        {
            if(!is_move_allowed_pawn(
                    piece, from, to, to_piece_id, out_remove_piece_id, out_msg))
            {
                assert(*out_msg != NULL);
                assert(*out_remove_piece_id == 0); // Although does not matter.
                return false;
            }
            break;
        }
        case mt_chess_type_knight:
        {
            if(!is_move_allowed_knight(from, to, out_msg))
            {
                assert(*out_msg != NULL);
                assert(*out_remove_piece_id == 0); // Although does not matter.
                return false;
            }
            break;
        }
        case mt_chess_type_bishop:
        {
            if(!is_move_allowed_bishop(from, to, out_msg))
            {
                assert(*out_msg != NULL);
                assert(*out_remove_piece_id == 0); // Although does not matter.
                return false;
            }
            break;  
        }
        case mt_chess_type_rook:
        {
            if(!is_move_allowed_rook(from, to, out_msg))
            {
                assert(*out_msg != NULL);
                assert(*out_remove_piece_id == 0); // Although does not matter.
                return false;
            }
            break;
        }
        case mt_chess_type_queen:
        {
            if(!is_move_allowed_queen(from, to, out_msg))
            {
                assert(*out_msg != NULL);
                assert(*out_remove_piece_id == 0); // Although does not matter.
                return false;
            }
            break;
        }

        default:
        {
            assert(false); // Must not get here.
            *out_msg = "Error: Unsupported piece type!";
            return false;
        }
    }

    // TODO: Check and return false, if move would result in check of own king
    //       (do not forget rook's position change on castling)!

    if(*out_remove_piece_id == 0)
    {
        *out_remove_piece_id = to_piece_id; // Although unnecessary, here.
    }
    //
    // Otherwise: Already set after valid "en passant" detection.

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
    uint8_t remove_piece_id = 0;

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
    
    if(!is_move_allowed(piece, &from, &to, &remove_piece_id, out_msg))
    {
        assert(remove_piece_id == 0); // Although does not matter, here.
        assert(*out_msg != NULL);
        return false;
    }
    
    s_data->board[piece_board_index] = 0;
    
    int const to_board_index = ((int)mt_chess_col_h + 1) * to.row + to.col;
    assert(0 <= to_board_index && to_board_index < 8 * 8);
    
    // E.g. for "en passant":
    if(remove_piece_id != 0 && remove_piece_id != s_data->board[to_board_index])
    {
        for(int i = 0; i < 8 * 8; ++i) // Overdone, if for "en passant", only.
        {
            if(s_data->board[i] == remove_piece_id)
            {
                s_data->board[i] = 0;
            }
        }
    }

    s_data->board[to_board_index] = piece->id;
    
    // TODO: Also move rook, if castling move detected!

    // Log:
    //
    {
        struct mt_chess_log_node * const node = mt_chess_log_node_create();

        node->move.from = from;
        node->move.to = to;
        node->move.piece = *piece;

        if(s_data->log == NULL)
        {
            s_data->log = node; // First move to be logged.
        }
        else
        {
            
            struct mt_chess_log_node * const latest_node =
                mt_chess_log_node_get_latest(s_data->log);

            assert(latest_node != NULL);
            assert(latest_node->next == NULL);

            latest_node->next = node;
            node->last = latest_node;
        }
    }

    s_data->turn = (enum mt_chess_color)(1 - (int)s_data->turn);

    assert(*out_msg == NULL);
    return true;
}