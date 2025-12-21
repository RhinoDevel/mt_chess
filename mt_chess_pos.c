
// Marcel Timm, RhinoDevel, 2025dec21

#ifdef __cplusplus
    #include <cassert>
    #include <cstddef>
    #include <cstdint>
#else //__cplusplus
    #include <assert.h>
    #include <stddef.h>
    #include <stdint.h>
#endif //__cplusplus

#include "mt_chess_pos.h"

static uint8_t const s_invalid_col = 0xFF;
static uint8_t const s_invalid_row = 0xFF;

void mt_chess_pos_invalidate(struct mt_chess_pos * const pos)
{
    assert(pos != NULL);
    
    pos->col = s_invalid_col;
    pos->row = s_invalid_row;
}
