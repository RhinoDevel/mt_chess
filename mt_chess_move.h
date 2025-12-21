
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

struct mt_chess_move
{
    // Must be a deep-copy, because of possible pawn promotion after this move.
    struct mt_chess_piece piece;
    
    struct mt_chess_pos from;
    struct mt_chess_pos to;
};

void mt_chess_move_invalidate(struct mt_chess_move * const move);

#endif //MT_CHESS_MOVE
