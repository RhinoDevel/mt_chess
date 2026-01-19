
// Marcel Timm, RhinoDevel, 2025dec28

#ifdef __cplusplus
    #include <cassert>
    #include <cstdint>
    #include <cstddef>
    #include <cstdbool>
#else //__cplusplus
    #include <assert.h>
    #include <stdint.h>
    #include <stddef.h>
    #include <stdbool.h>
#endif //__cplusplus

#include "mt_chess_attack.h"
#include "mt_chess_piece.h"
#include "mt_chess_color.h"
#include "mt_chess_row.h"
#include "mt_chess_col.h"
#include "mt_chess_type.h"
#include "mt_chess_log_node.h"

// TODO: Maybe better implement this as macro for performance-improvement!
//
/**
 * - Under attack, if empty square or occupied by defending player.
 * - Ray is stopped, if occupied by any kind of piece.
 * 
 * - Additionally returns, if the ray is stopped at current index or not.
 */
static bool ray_update_attack_map(
    struct mt_chess_piece const * const pieces,
    struct mt_chess_piece const * const piece, // Attacking piece (ray's src.).
    int const cur_index, // Board & attack map index of current square in ray.
    uint8_t const cur_piece_id, // ID of piece at current square (0, if empty).
    uint8_t * const attack_map)
{
    if(cur_piece_id == 0)
    {
        // There is NO piece at current square.
        attack_map[cur_index] = 1; // (maybe already set, does not matter)
        return false; // The ray is NOT stopped by this (empty) square.
    }

    // There is some piece at current square.

    int const cur_piece_index = MT_CHESS_PIECE_GET_INDEX_BY_ID(cur_piece_id);
    struct mt_chess_piece const * const cur_piece = pieces + cur_piece_index;

    if(cur_piece->color != piece->color)
    {
        // Defending player's piece at current position.
        attack_map[cur_index] = 1; // (maybe already set, does not matter)
    }
    //
    // Otherwise: Attacker's piece at current position. <=> NOT attacked.
    
    return true; // The ray is stopped by this square.
}

// TODO: Maybe better implement this as macro for performance-improvement!
//
/**
 * - It is expected that row and/or column given are invalid (out of bounds).
 */
static void row_and_col_update_attack_map(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_piece const * const piece, // Attacking piece (ray's src.).
    int const row,
    int const col,
    uint8_t * const attack_map)
{
    if(0 <= row && row <= (int)mt_chess_row_1
        && 0 <= col && col <= (int)mt_chess_col_h)
    {
        int const cur_index = row * ((int)mt_chess_row_1 + 1) + col;

        ray_update_attack_map( // Ignoring return value, here.
            pieces,
            piece,
            cur_index,
            board[cur_index],
            attack_map);
    }
}

static void add_to_attack_map_king(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_piece const * const piece,
    int const piece_row,
    int const piece_col,
    int const index,
    uint8_t * const attack_map)
{
    assert(piece->type == mt_chess_type_king);
    
    int const row_min = piece_row == 0 
            ? 0 : piece_row - 1;
    int const row_max = piece_row == (int)mt_chess_row_1
            ? (int)mt_chess_row_1 : piece_row + 1;

    int const col_min = piece_col == 0
            ? 0 : piece_col - 1;
    int const col_max = piece_col == (int)mt_chess_col_h
            ? (int)mt_chess_col_h : piece_col + 1;

    for(int row = row_min; row <= row_max; ++row)
    {
        int const row_offset = row * ((int)mt_chess_row_1 + 1);

        for(int col = col_min; col <= col_max; ++col)
        {
            int const cur_index = row_offset + col;

            if(cur_index == index)
            {
                assert(row == piece_row && col == piece_col);
                assert(attack_map[cur_index] == 0);
                continue; // Ignore king's position.
            }
            assert(!(row == piece_row && col == piece_col));

            uint8_t const cur_piece_id = board[cur_index];

            ray_update_attack_map( // Ignoring return value.
                pieces, piece, cur_index, cur_piece_id, attack_map);
        }
    }
}

static void add_to_attack_map_pawn(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_piece const * const piece,
    struct mt_chess_log_node const * const latest_move,
    int const piece_row,
    int const piece_col,
    uint8_t * const attack_map)
{
    assert(piece->type == mt_chess_type_pawn);

    bool is_en_passant = false;

    //   0 1 2 
    // 0| | | |
    //  -------
    // 1|x| |x|
    //  -------
    // 2|!|p|!|

    do
    {
        if(latest_move == NULL)
        {
            break; // This is the first move. => No "en passant" possible.
        }

        if(latest_move->move.piece.type != mt_chess_type_pawn)
        {
            break; // Preceding move was NOT made by a pawn. => No "en passant".
        }

        int const en_passant_row = (int)(piece->color == mt_chess_color_white
                ? mt_chess_row_5 : mt_chess_row_4);

        if(piece_row != en_passant_row)
        {
            break; // "En passant" not possible in attacking piece's row.
        }

        if((int)latest_move->move.to.row != en_passant_row)
        {
            break;
        }
        
        int const en_passant_src_row =
                (int)(piece->color == mt_chess_color_white
                    ? mt_chess_row_7 : mt_chess_row_2);

        if((int)latest_move->move.from.row != en_passant_src_row)
        {
            break; // Was not the first step of the defender's pawn.
        }

        assert(0 <= piece_col && piece_col <= (int)mt_chess_col_h);
        assert(latest_move->move.to/*from*/.col <= (uint8_t)mt_chess_col_h);

        if((int)latest_move->move.to/*from*/.col != piece_col + 1
            && (int)latest_move->move.to/*from*/.col != piece_col - 1)
        {
            break; // Defender's pawn is not on a neighboring file.
        }

        is_en_passant = true;
    } while(false);

    // For the non-en-passant/default pawn attack(-s).
    int const row = piece_row + (int)(piece->color == mt_chess_color_white
            ?  -1 : 1); // (from the top)

    if(!(0 <= row && row <= (int)mt_chess_row_1))
    {
        return; // Nothing under attack by the pawn.
    }

    if(is_en_passant)
    {
        int const en_passant_row = piece_row;
        int const en_passant_col = (int)latest_move->move.to/*from*/.col;
        int const en_passant_index =
            en_passant_row * ((int)mt_chess_row_1 + 1) + en_passant_col;

        assert(board[en_passant_index] == latest_move->move.piece.id);

        attack_map[en_passant_index] = 1;

        // The square right above/below the en-passant-index square, which would
        // normally be under attack, is considered as NOT under attack, here,
        // because it must be empty (as the defender's pawn just moved through
        // that square, from its initial position).

        // On the other side, the "normal" pawn attack is valid:
        row_and_col_update_attack_map(
            pieces,
            board,
            piece,
            row,
            piece_col + (en_passant_col < piece_col ? 1 : -1),
            attack_map);
        return;
    }

    // No "en passant":

    row_and_col_update_attack_map(
        pieces, board, piece, row, piece_col - 1, attack_map);
    row_and_col_update_attack_map(
        pieces, board, piece, row, piece_col + 1, attack_map);
}

static void add_to_attack_map_knight(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_piece const * const piece,
    int const piece_row,
    int const piece_col,
    uint8_t * const attack_map)
{
    assert(piece->type == mt_chess_type_knight);

    //   0 1 2 3 4
    //  -----------
    // 0| |C| |B| |
    //  -----------
    // 1|D| | | |A|
    //  -----------
    // 2| | |n| | |
    //  -----------
    // 3|E| | | |H|
    //  -----------
    // 4| |F| |G| |

    row_and_col_update_attack_map( // A
        pieces, board, piece, piece_row - 1, piece_col + 2, attack_map);
    row_and_col_update_attack_map( // B
        pieces, board, piece, piece_row - 2, piece_col + 1, attack_map);
    row_and_col_update_attack_map( // C
        pieces, board, piece, piece_row - 2, piece_col - 1, attack_map);
    row_and_col_update_attack_map( // D
        pieces, board, piece, piece_row - 1, piece_col - 2, attack_map);
    row_and_col_update_attack_map( // E
        pieces, board, piece, piece_row + 1, piece_col - 2, attack_map);
    row_and_col_update_attack_map( // F
        pieces, board, piece, piece_row + 2, piece_col - 1, attack_map);
    row_and_col_update_attack_map( // G
        pieces, board, piece, piece_row + 2, piece_col + 1, attack_map);
    row_and_col_update_attack_map( // H
        pieces, board, piece, piece_row + 1, piece_col + 2, attack_map);
}

/**
 * - Also called by add_attack_map_queen(). 
 */
static void add_to_attack_map_bishop(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_piece const * const piece,
    int const piece_row,
    int const piece_col,
    uint8_t * const attack_map)
{
    assert(piece->type == mt_chess_type_bishop
        || piece->type == mt_chess_type_queen); // <- Is reused.

    int col = 0;
    int row = 0;

    // ****************
    // *** Up-left: ***
    // ****************

    row = piece_row;
    col = piece_col;
    while(0 <= --row && 0 <= --col)
    {
        int const row_offset = row * ((int)mt_chess_col_h + 1);
        int const cur_index = row_offset + col;
        uint8_t const cur_piece_id = board[cur_index];

        if(ray_update_attack_map(
            pieces, piece, cur_index, cur_piece_id, attack_map))
        {
            assert(cur_piece_id != 0);
            break; // Ray is stopped by current square.
        }
        assert(cur_piece_id == 0);
    }

    // *******************
    // *** Down-right: ***
    // *******************

    row = piece_row;
    col = piece_col;
    while(++row <= (int)mt_chess_row_1 && ++col <= (int)mt_chess_col_h)
    {
        int const row_offset = row * ((int)mt_chess_col_h + 1);
        int const cur_index = row_offset + col;
        uint8_t const cur_piece_id = board[cur_index];

        if(ray_update_attack_map(
            pieces, piece, cur_index, cur_piece_id, attack_map))
        {
            assert(cur_piece_id != 0);
            break; // Ray is stopped by current square.
        }
        assert(cur_piece_id == 0);
    }

    // ******************
    // *** Down-left: ***
    // ******************

    row = piece_row;
    col = piece_col;
    while(++row <= (int)mt_chess_row_1 && 0 <= --col)
    {
        int const row_offset = row * ((int)mt_chess_col_h + 1);
        int const cur_index = row_offset + col;
        uint8_t const cur_piece_id = board[cur_index];

        if(ray_update_attack_map(
            pieces, piece, cur_index, cur_piece_id, attack_map))
        {
            assert(cur_piece_id != 0);
            break; // Ray is stopped by current square.
        }
        assert(cur_piece_id == 0);
    }

    // *****************
    // *** Up-right: ***
    // *****************

    row = piece_row;
    col = piece_col;
    while(0 <= --row && ++col <= (int)mt_chess_col_h)
    {
        int const row_offset = row * ((int)mt_chess_col_h + 1);
        int const cur_index = row_offset + col;
        uint8_t const cur_piece_id = board[cur_index];

        if(ray_update_attack_map(
            pieces, piece, cur_index, cur_piece_id, attack_map))
        {
            assert(cur_piece_id != 0);
            break; // Ray is stopped by current square.
        }
        assert(cur_piece_id == 0);
    }
}

/**
 * - Also called by add_attack_map_queen().
 */
static void add_to_attack_map_rook(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_piece const * const piece,
    int const piece_row,
    int const piece_col,
    uint8_t * const attack_map)
{
    assert(piece->type == mt_chess_type_rook
        || piece->type == mt_chess_type_queen); // <- Is reused.

    int col = 0;
    int row = 0;

    // ***********
    // *** Up: ***
    // ***********

    row = piece_row;
    while(0 <= --row)
    {
        int const row_offset = row * ((int)mt_chess_col_h + 1);
        int const cur_index = row_offset + piece_col;
        uint8_t const cur_piece_id = board[cur_index];

        if(ray_update_attack_map(
            pieces, piece, cur_index, cur_piece_id, attack_map))
        {
            assert(cur_piece_id != 0);
            break; // Ray is stopped by current square.
        }
        assert(cur_piece_id == 0);
    }

    // *************
    // *** Down: ***
    // *************

    row = piece_row;
    while(++row <= (int)mt_chess_row_1)
    {
        int const row_offset = row * ((int)mt_chess_col_h + 1);
        int const cur_index = row_offset + piece_col;
        uint8_t const cur_piece_id = board[cur_index];

        if(ray_update_attack_map(
            pieces, piece, cur_index, cur_piece_id, attack_map))
        {
            assert(cur_piece_id != 0);
            break; // Ray is stopped by current square.
        }
        assert(cur_piece_id == 0);
    }

    // ***

    int const piece_row_offset = piece_row * ((int)mt_chess_col_h + 1);

    // *************
    // *** Left: ***
    // *************

    col = piece_col;
    while(0 <= --col)
    {
        int const cur_index = piece_row_offset + col;
        uint8_t const cur_piece_id = board[cur_index];

        if(ray_update_attack_map(
            pieces, piece, cur_index, cur_piece_id, attack_map))
        {
            assert(cur_piece_id != 0);
            break; // Ray is stopped by current square.
        }
        assert(cur_piece_id == 0);
    }

    // **************
    // *** Right: ***
    // **************

    col = piece_col;
    while(++col <= (int)mt_chess_col_h)
    {
        int const cur_index = piece_row_offset + col;
        uint8_t const cur_piece_id = board[cur_index];

        if(ray_update_attack_map(
            pieces, piece, cur_index, cur_piece_id, attack_map))
        {
            assert(cur_piece_id != 0);
            break; // Ray is stopped by current square.
        }
        assert(cur_piece_id == 0);
    }
}

static void add_to_attack_map_queen(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_piece const * const piece,
    int const piece_row,
    int const piece_col,
    uint8_t * const attack_map)
{
    add_to_attack_map_bishop(
        pieces, board, piece, piece_row, piece_col, attack_map);
    add_to_attack_map_rook(
        pieces, board, piece, piece_row, piece_col, attack_map);
}

static void add_to_attack_map(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_piece const * const piece,
    struct mt_chess_log_node const * const latest_move,
    int const piece_row,
    int const piece_col,
    int const index,
    uint8_t * const attack_map)
{
    assert(piece != NULL);
    assert(0 <= piece_row && piece_row <= (int)mt_chess_row_1);
    assert(0 <= piece_col && piece_col <= (int)mt_chess_col_h);
    assert(0 <= index && index < 8 * 8);
    assert(attack_map != NULL);
    assert(attack_map[index] == 0);

    switch(piece->type)
    {
        case mt_chess_type_king:
        {
            add_to_attack_map_king(
                pieces, board, piece, piece_row, piece_col, index, attack_map);
            return;
        }
        case mt_chess_type_pawn:
        {
            add_to_attack_map_pawn(
                pieces,
                board,
                piece,
                latest_move,
                piece_row,
                piece_col,
                attack_map);
            return;
        }
        case mt_chess_type_knight:
        {
            add_to_attack_map_knight(
                pieces, board, piece, piece_row, piece_col, attack_map);
            return;
        }
        case mt_chess_type_bishop:
        {
            add_to_attack_map_bishop(
                pieces, board, piece, piece_row, piece_col, attack_map);
            return;
        }
        case mt_chess_type_rook:
        {
            add_to_attack_map_rook(
                pieces, board, piece, piece_row, piece_col, attack_map);
            return;
        }
        case mt_chess_type_queen:
        {
            add_to_attack_map_queen(
                pieces, board, piece, piece_row, piece_col, attack_map);
            return;
        }

        default:
        {
            assert(false); // Must not get here!
            return;
        }
    }
}

void mt_chess_attack_update(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_log_node const * const latest_move,
    enum mt_chess_color const attacker,
    uint8_t * const attack_map)
{
    assert(pieces != NULL);
    assert(board != NULL);
    // latest_move may be NULL.
    assert(
        attacker == mt_chess_color_white || attacker == mt_chess_color_black);
    assert(attack_map != NULL);

    // Reset the attack map:
    for(int index = 0; index < 8 * 8; ++index)
    {
        attack_map[index] = 0;
    }

    // Check each square of the board for a piece. If found, determine which
    // other squares are attacked by that piece and mark these "squares" on the
    // attack map:
    for(int row = (int)mt_chess_row_8; row <= (int)mt_chess_row_1; ++row)
    {
        int const row_offset = row * ((int)mt_chess_col_h + 1);

        for(int col = (int)mt_chess_col_a; col <= (int)mt_chess_col_h; ++col)
        {
            int const index = row_offset + col; // For board & attack map.
            uint8_t const piece_id = board[index];

            if(piece_id == 0)
            {
                continue; // Current square is empty. => Does not attack anyone.
            }

            // There is a piece at the current square of the board.

            int const piece_index = MT_CHESS_PIECE_GET_INDEX_BY_ID(piece_id);

            struct mt_chess_piece const * const piece = pieces + piece_index;
            
            if(piece->color != attacker)
            {
                // One of the pieces of the player having the current turn.
                continue;
            }

            // The piece belongs to the attacker.

            add_to_attack_map(
                pieces, board, piece, latest_move, row, col, index, attack_map);
        }
    }
}