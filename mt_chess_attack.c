
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

static void add_to_attack_map(
    struct mt_chess_piece const * const piece,
    int const piece_row,
    int const piece_col,
    uint8_t * const attack_map)
{
    assert(piece != NULL);
    assert(0 <= piece_row && piece_row <= (int)mt_chess_row_1);
    assert(0 <= piece_col && piece_col <= (int)mt_chess_col_h);
    assert(attack_map != NULL);

    switch(piece->type)
    {
        case mt_chess_type_king:
        {
            // TODO: Implement!
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
            // TODO: Implement!
            return;
        }
        case mt_chess_type_rook:
        {
            // TODO: Implement!
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

            add_to_attack_map(piece, row, col, attack_map);
        }
    }
}