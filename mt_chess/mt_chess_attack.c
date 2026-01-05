
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
    if(attack_map[cur_index] != 0)
    {
        // Square is already marked as being attacked.
        return false; // Ray can continue.
    }

    // Square is not yet marked as being under attack.

    if(cur_piece_id == 0)
    {
        // There is NO piece at current square.
        attack_map[cur_index] = 1;
        return false; // The ray is NOT stopped by this (empty) square.
    }

    // There is some piece at current square.

    int const cur_piece_index = mt_chess_piece_get_index(pieces, cur_piece_id);
    struct mt_chess_piece const * const cur_piece = pieces + cur_piece_index;

    if(cur_piece->color != piece->color)
    {
        // Defending player's piece at current position.
        attack_map[cur_index] = 1;
    }
    //
    // Otherwise: Attacker's piece at current position. <=> NOT attacked.
    
    return true; // The ray is stopped by this square.
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

static void add_to_attack_map_bishop(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_piece const * const piece,
    int const piece_row,
    int const piece_col,
    int const index,
    uint8_t * const attack_map)
{
    assert(attack_map[index] == 0);
    assert(piece->type == mt_chess_type_bishop);

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

static void add_to_attack_map_rook(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_piece const * const piece,
    int const piece_row,
    int const piece_col,
    int const index,
    uint8_t * const attack_map)
{
    assert(attack_map[index] == 0);
    assert(piece->type == mt_chess_type_rook);

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

static void add_to_attack_map(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    struct mt_chess_piece const * const piece,
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
            // TODO: Implement!
            return;
        }
        case mt_chess_type_knight:
        {
            // TODO: Implement!
            return;
        }
        case mt_chess_type_bishop:
        {
            add_to_attack_map_bishop(
                pieces, board, piece, piece_row, piece_col, index, attack_map);
            return;
        }
        case mt_chess_type_rook:
        {
            add_to_attack_map_rook(
                pieces, board, piece, piece_row, piece_col, index, attack_map);
            return;
        }
        case mt_chess_type_queen:
        {
            // TODO: Implement!
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
    enum mt_chess_color const attacker,
    uint8_t * const attack_map)
{
    assert(pieces != NULL);
    assert(board != NULL);
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

            int const piece_index = mt_chess_piece_get_index(pieces, piece_id);

            struct mt_chess_piece const * const piece = pieces + piece_index;
            
            if(piece->color != attacker)
            {
                // One of the pieces of the player having the current turn.
                continue;
            }

            // The piece belongs to the attacker.

            add_to_attack_map(
                pieces, board, piece, row, col, index, attack_map);
        }
    }
}