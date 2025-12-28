
// Marcel Timm, RhinoDevel, 2025dec21

#ifdef __cplusplus
    #include <cassert>
    #include <cstddef>
    #include <cstdint>
    #include <cstdlib>
#else //__cplusplus
    #include <assert.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <stdlib.h>
#endif //__cplusplus

#include "mt_chess_move.h"
#include "mt_chess_type.h"
#include "mt_chess_col.h"
#include "mt_chess_row.h"

void mt_chess_move_invalidate(struct mt_chess_move * const move)
{
    assert(move != NULL);
    
    mt_chess_piece_invalidate(&move->piece);
    
    mt_chess_pos_invalidate(&move->from);
    mt_chess_pos_invalidate(&move->to);
}

void mt_chess_move_apply(
    struct mt_chess_move const * const move, uint8_t * const board)
{
    assert(move != NULL);
    assert(board != NULL);

    int const from_index =
            move->from.row * ((int)mt_chess_col_h + 1) + move->from.col;
    assert(0 <= from_index && from_index < 8 * 8);

    int const to_index =
            move->to.row * ((int)mt_chess_col_h + 1) + move->to.col;
    assert(0 <= to_index && to_index < 8 * 8);

    assert(board[from_index] == move->piece.id);

    // NOT doing the move, yet (see below).

    if(move->piece.type == mt_chess_type_king)
    {
        // Additionally move rook, if castling.

        int const horiz_dist_val = move->to.col - move->from.col;

        if(abs(horiz_dist_val) == 2)
        {
            assert(board[to_index] == 0); // (not updated, yet)

            // Initial values are for kingside castling:
            enum mt_chess_col rook_from_col = mt_chess_col_h;
            enum mt_chess_col rook_to_col = mt_chess_col_f;

            if(horiz_dist_val < 0)
            {
                // Queenside castling.
                rook_from_col = mt_chess_col_a;
                rook_to_col = mt_chess_col_d;
            }

            int const rook_from_index =
                move->from/*to*/.row * ((int)mt_chess_col_h + 1)
                    + rook_from_col;
             assert(0 <= rook_from_index && rook_from_index < 8 * 8);

            int const rook_to_index =
                move->to/*from*/.row * ((int)mt_chess_col_h + 1) + rook_to_col;
            assert(0 <= rook_to_index && rook_to_index < 8 * 8);

            uint8_t const rook_piece_id = board[rook_from_index];

            assert(rook_piece_id != 0); // Kind of a parity test..
            assert(board[rook_to_index] == 0);

            // Move rook:
            board[rook_from_index] = 0;
            board[rook_to_index] = move->piece.id;
        }
    }
    else 
    {
        if(move->piece.type == mt_chess_type_pawn)
        {
            if(move->from.col != move->to.col)
            {
                // Assuming "en passant".

                assert(board[to_index] == 0); // (not updated, yet)

                uint8_t const opponent_pawn_row = move->from.row;
                uint8_t const opponent_pawn_col = move->to.col;

                int const opponent_pawn_index =
                        opponent_pawn_row * ((int)mt_chess_col_h + 1)
                            + opponent_pawn_col;
                assert(0 <= opponent_pawn_index && opponent_pawn_index < 8 * 8);

                // Kind of a parity test..
                assert(board[opponent_pawn_index] != 0);
                
                // Remove opponent's pawn from board:
                board[opponent_pawn_index] = 0;
            }
        }
    }

    // Move the piece:
    board[from_index] = 0;
    board[to_index] = move->piece.id;
}
