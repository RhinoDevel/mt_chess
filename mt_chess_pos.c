
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
#include "mt_chess_col.h"
#include "mt_chess_row.h"

static uint8_t const s_invalid_col = 0xFF;
static uint8_t const s_invalid_row = 0xFF;

void mt_chess_pos_invalidate(struct mt_chess_pos * const pos)
{
    assert(pos != NULL);
    
    pos->col = s_invalid_col;
    pos->row = s_invalid_row;
}

bool mt_chess_pos_is_invalid(struct mt_chess_pos const * const pos)
{
    assert((pos->col == s_invalid_col) == (pos->row == s_invalid_row));
    return pos->col == s_invalid_col || pos->row == s_invalid_row;
}

struct mt_chess_pos mt_chess_pos_get(char const file, char const rank)
{
    struct mt_chess_pos ret_val;
    
    mt_chess_pos_invalidate(&ret_val);
    
    if(file < 'a' || 'h' < file)
    {
        return ret_val; // Is invalid.
    }
    if(rank < '1' || '8' < rank)
    {
        return ret_val; // Is invalid.
    }
    
    ret_val.col = (uint8_t)((enum mt_chess_col)(file - 'a'));
    assert(0 <= ret_val.col && ret_val.col <= 7);
    
    ret_val.row =
        (uint8_t)((enum mt_chess_row)((int)mt_chess_row_1 - (int)(rank - '1')));
    assert(0 <= ret_val.row && ret_val.row <= 7);
    
    return ret_val;
}
