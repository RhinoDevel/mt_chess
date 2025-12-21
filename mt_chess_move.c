
// Marcel Timm, RhinoDevel, 2025dec21

#ifdef __cplusplus
    #include <cassert>
    #include <cstddef>
#else //__cplusplus
    #include <assert.h>
    #include <stddef.h>
#endif //__cplusplus

#include "mt_chess_move.h"

void mt_chess_move_invalidate(struct mt_chess_move * const move)
{
    assert(move != NULL);
    
    mt_chess_piece_invalidate(&move->piece);
    
    mt_chess_pos_invalidate(&move->from);
    mt_chess_pos_invalidate(&move->to);
}
