
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
#include "mt_chess_move.h"
#include "mt_chess_str.h"

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

    // Is the to-square horizont. equal, adjacent or 2 squares away from origin?

    int const horiz_dist_val = (int)to->col - (int)from->col;
    int const horiz_dist = abs(horiz_dist_val);

    if(2 < horiz_dist)
    {
        // Neither a "normal", nor a castling move.
        *out_msg = "A king can move at most two files per turn.";
        return false;
    }

    if(horiz_dist == 2)
    {
        // Could be castling.

        if(vert_dist != 0)
        {
            *out_msg = "A king can never move two files and rank(-s) at once.";
            return false;
        }

        // Moving two squares on a row/rank.
        assert(vert_dist == 0 && horiz_dist == 2);

        // Is the straight horizontal two square move allowed on current rank?

        uint8_t const start_row = s_data->turn == mt_chess_color_white
            ? (uint8_t)mt_chess_row_1 : (uint8_t)mt_chess_row_8;

        if(from/*to*/->row != start_row)
        {
            *out_msg = "A king can move two files at most on the start row.";
            return false;
        }

        // Get king's piece object:

        int const board_index_king =
                from->row * ((int)mt_chess_col_h + 1) + from->col;
        assert(0 <= board_index_king && board_index_king < 8 * 8);

        uint8_t const piece_id_king = s_data->board[board_index_king];

        int const piece_index_king = mt_chess_piece_get_index(
                s_data->pieces, piece_id_king);

        struct mt_chess_piece const * const piece_king =
            &s_data->pieces[piece_index_king];

        assert(piece_king->type == mt_chess_type_king);
        assert(piece_king->color == s_data->turn);

        // Is this the king's initial move?
        
        // Not performance-optimized (but should be OK, here):
        struct mt_chess_log_node * const last_log_node_king =
            mt_chess_log_node_get_latest_of_piece(s_data->log, piece_king->id);

        if(last_log_node_king != NULL)
        {
            assert(last_log_node_king->move.piece.id == piece_king->id);
            assert(last_log_node_king->move.piece.type == mt_chess_type_king);
            assert(last_log_node_king->move.piece.color = s_data->turn);
            assert(mt_chess_pos_are_equal(&last_log_node_king->move.to, from));

            *out_msg = "This is not the king's first move, castling not possible.";
            return false;
        }

        // Get rook's position required for castling:

        static uint8_t const rook_col_long = (uint8_t)mt_chess_col_a;
        static uint8_t const rook_col_short = (uint8_t)mt_chess_col_h;
        // (these are independent of color, because both queens are on file D)

        int const rook_row_offset = from/*to*/->row * ((int)mt_chess_col_h + 1);

        int const rook_col = 0 < horiz_dist_val
                ? rook_col_short // Move to kingside.
                : rook_col_long; // Move to queenside.

        int const board_index_rook =
                rook_row_offset * ((int)mt_chess_col_h + 1) + rook_col;
        assert(0 <= board_index_rook && board_index_rook < 8 * 8);

        // Get what may be the rook's piece ID, from the board:

        uint8_t const board_rook_piece_id = s_data->board[board_index_rook];

        // Is there a piece on the rook position necessary for castling?

        if(board_rook_piece_id == 0)
        {
            if(rook_col == rook_col_short)
            {
                *out_msg = "Rook missing for kingside castling.";
                return false;
            }
            assert(rook_col == rook_col_long);
            *out_msg = "Rook missing for queenside castling.";
            return false;
        }

        // Get object of piece that resides at the rook castling square:

        int const rook_piece_index = mt_chess_piece_get_index(
                s_data->pieces, board_rook_piece_id);
        assert(0 < rook_piece_index);

        struct mt_chess_piece const * const rook_piece =
            s_data->pieces + rook_piece_index;
        assert(rook_piece->id == board_rook_piece_id);

        // Is it actually a rook at the position for castling?

        if(rook_piece->type != mt_chess_type_rook)
        {
            if(rook_col == rook_col_short)
            {
                *out_msg = "The piece at kingside rook castling square is not a rook.";
                return false;
            }
            assert(rook_col == rook_col_long);
            *out_msg = "The piece at queenside rook castling square is not a rook.";
            return false;
        }

        // Is the rook at the castling position the current player's?

        if(rook_piece->color != s_data->turn)
        {
            if(rook_col == rook_col_short)
            {
                *out_msg = "There is an opponent's rook at kingside castling square.";
                return false;
            }
            assert(rook_col == rook_col_long);
            *out_msg = "There is an opponent's rook at queenside castling square.";
            return false;
        }

        // Did the rook not move (on its own), yet?

        // (this code is OK, even, if rook was implicitly moved before via
        //  already done castling, because then the kind's-first-move check
        //  will fail)

        // Not performance-optimized (but should be OK, here):
        struct mt_chess_log_node * const last_log_node_rook =
            mt_chess_log_node_get_latest_of_piece(
                s_data->log, rook_piece->id);

        if(last_log_node_rook != NULL)
        {
            assert(last_log_node_rook->move.piece.id == rook_piece->id);
            assert(
                last_log_node_rook->move.piece.type == mt_chess_type_rook);
            assert(last_log_node_rook->move.piece.color = s_data->turn);
            assert(
                last_log_node_rook->move.to.row == rook_col
                    && last_log_node_rook->move.to.col == from/*to*/->row);

            *out_msg = "This rook was already moved, castling not possible.";
            return false;
        }

        // Verify that all squares are empty that need to be empty for castling:

        if(horiz_dist_val == 2)
        {
            // Kingside castling attempt.

            // Is the square of file F empty?

            if(s_data->board[board_index_king + 1] != 0)
            {
                *out_msg = "Square of file F is not empty, kingside castling not possible.";
                return false;
            }
        
            // Is the square of file G empty?

            if(s_data->board[board_index_king + 2] != 0)
            {
                *out_msg = "Square of file G is not empty, kingside castling not possible.";
                return false;
            }
        }
        else
        {
            // Queenside castling attempt.
            assert(horiz_dist_val == -2);

            // Is the square of file B empty?
            
            if(s_data->board[board_index_rook + 1] != 0)
            {
                *out_msg = "Square of file B is not empty, queenside castling not possible.";
                return false;
            }

            // Is the square of file C empty?

            if(s_data->board[board_index_rook + 2] != 0)
            {
                *out_msg = "Square of file C is not empty, queenside castling not possible.";
                return false;
            }

            // Is the square of file D empty?

            if(s_data->board[board_index_rook + 3] != 0)
            {
                *out_msg = "Square of file D is not empty, queenside castling not possible.";
                return false;
            }
        }

        // TODO:
        // - Is the king not attacked on the from-square?
        // - Is the square the king crosses not attacked?
        // [- Not necessary to check, here: Is to-square not attacked?]

        assert(*out_msg == NULL);
        return true;
    }

    // NOT castling.

    assert(vert_dist <= 1 && horiz_dist <= 1);
    assert((vert_dist == 0) != (horiz_dist == 0));

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

    int col = 0;
    int row = 0;

    int const horiz_dist = abs(to->col - from->col);
    int const vert_dist = abs(to->row - from->row);

    if(horiz_dist != vert_dist)
    {
        *out_msg = "A bishop can move diagonally, only.";
        return false;
    }

    if(horiz_dist == 1)
    {
        // That there is at most one of the opponent's pieces on the destination
        // square must be checked elsewhere.

        assert(vert_dist == 1);

        assert(*out_msg == NULL);
        return true;
    }

    int const col_add = from->col < to->col ? 1 : -1;
    int const row_add = from->row < to->row ? 1 : -1;

    int const first_col = from->col + col_add;
    int const first_row = from->row + row_add;

    assert(0 <= first_col && first_col <= 7);
    assert(0 <= first_row && first_row <= 7);

    int const last_col = first_col + col_add * (horiz_dist - 2);
    int const last_row = first_row + row_add * (vert_dist - 2);

    assert(0 <= last_col && last_col <= 7);
    assert(0 <= last_row && last_row <= 7);

    col = first_col;
    row = first_row;
    do
    {
        int const row_offset = ((int)mt_chess_col_h + 1) * row;
        int const board_index = row_offset + col;
        assert(0 <= board_index && board_index < 8 * 8);

        if(s_data->board[board_index] != 0)
        {
            *out_msg = "There is at least one piece in the bishop's path.";
            return false;
        }

        if(row == last_row)
        {
            assert(col == last_col);
            break; // Done
        }

        row += row_add;
        col += col_add;
    } while(true);

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
    char const * * const out_msg)
{
    assert(piece != NULL && piece->id != 0);
    assert(from != NULL && !mt_chess_pos_is_invalid(from));
    assert(to != NULL && !mt_chess_pos_is_invalid(to));
    assert(out_msg != NULL);

    assert(*out_msg == NULL);

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
    return true; // Seems to be an OK move.
}

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

    struct mt_chess_piece const * to_piece = NULL;

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
                return false;
            }
            break;
        }
        case mt_chess_type_pawn:
        {
            if(!is_move_allowed_pawn(
                piece, from, to, to_piece_id, out_msg))
            {
                assert(*out_msg != NULL);
                return false;
            }
            break;
        }
        case mt_chess_type_knight:
        {
            if(!is_move_allowed_knight(from, to, out_msg))
            {
                assert(*out_msg != NULL);
                return false;
            }
            break;
        }
        case mt_chess_type_bishop:
        {
            if(!is_move_allowed_bishop(from, to, out_msg))
            {
                assert(*out_msg != NULL);
                return false;
            }
            break;
        }
        case mt_chess_type_rook:
        {
            if(!is_move_allowed_rook(from, to, out_msg))
            {
                assert(*out_msg != NULL);
                return false;
            }
            break;
        }
        case mt_chess_type_queen:
        {
            if(!is_move_allowed_queen(from, to, out_msg))
            {
                assert(*out_msg != NULL);
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

    // TODO: Check and return false, if king is under attack / in check after move (no matter, if this was already true before or would be caused by the suggested move)!

    assert(*out_msg == NULL);
    return true;
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

/**
 * - This is a wrapper.
 */
MT_EXPORT_CHESS_API char* __stdcall mt_chess_create_board_as_str(
    bool const unicode)
{
    if(s_data == NULL)
    {
        return NULL; // Not initialized, yet.
    }
    return mt_chess_str_create_board(s_data, unicode);
}

MT_EXPORT_CHESS_API bool __stdcall mt_chess_try_move(
    char const from_file, char const from_rank,
    char const to_file, char const to_rank,
    char const * * const out_msg)
{
    assert(out_msg != NULL);
    *out_msg = NULL;
    
    struct mt_chess_move move;
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

    move.piece = *piece;
    move.from = from;
    move.to = to;

    mt_chess_move_apply(&move, s_data->board);

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