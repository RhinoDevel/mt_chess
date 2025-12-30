
// Marcel Timm, RhinoDevel, 2025dec21

#ifndef MT_CHESS_MOVE
#define MT_CHESS_MOVE

#ifdef __cplusplus
    #include <cstdint>
#else //__cplusplus
    #include <stdint.h>
#endif //__cplusplus

#include "mt_chess_piece.h"
#include "mt_chess_pos.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/**
 * - Castling must be detected implicitly by the piece being a king and the move
 *   being a king-castling move.
 * 
 * - "En passant" must be detected implicitly by the piece being a pawn and the
 *   move being a pawn-"en passant" move.
 */
struct mt_chess_move
{
    // Must be a deep-copy, because of possible pawn promotion after this move.
    struct mt_chess_piece piece;
    
    struct mt_chess_pos from;
    struct mt_chess_pos to;
};

void mt_chess_move_invalidate(struct mt_chess_move * const move);

/**
 * - Detects castling and applies additional move of the rook implicitly.
 * - Detects "en passant" and removes other pawn automatically.
 * - Does no error handling/detection! Assumes, given move is pseudo-possible
 *   [meaning: Move must have been validated by mt_chess/is_move_allowed(), but
 *    we are ignoring check, check-mate and pinning rules that may forbid this
 *    move, here].
 */
void mt_chess_move_apply(
    struct mt_chess_move const * const move, uint8_t * const board);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //MT_CHESS_MOVE
