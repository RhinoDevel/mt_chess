
// Marcel Timm, RhinoDevel, 2025dec21

#ifdef __cplusplus
    #include <cassert>
    #include <cstddef>
    #include <cstdint>
    #include <cstdbool>
#else //__cplusplus
    #include <assert.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <stdbool.h>
#endif //__cplusplus

#include "mt_chess_piece.h"
#include "mt_chess_color.h"
#include "mt_chess_type.h"

int mt_chess_piece_get_index_by_type_and_color(
    struct mt_chess_piece const * const pieces,
    enum mt_chess_type const type,
    enum mt_chess_color const color)
{
    assert(pieces != NULL);
    // Also assuming that array the pointer points to has 2 x 2 x 8 items.
    // Assuming that type & color given are valid, but no problem here, if not.

    for(int i = 0; i < 2 * 2 * 8; ++i)
    {
        if(pieces[i].color != color)
        {
            continue;
        }
        if(pieces[i].type != type)
        {
            continue;
        }
        return i;
    }
    assert(false);
    return -1; // Not found.
}

void mt_chess_piece_init(struct mt_chess_piece * const pieces)
{
    assert(pieces != NULL);
    // Also assuming that array the pointer points to has 2 x 2 x 8 items.
    
    int i = 0; // Per piece.
    
    assert((int)mt_chess_color_white == 0);
    assert((int)mt_chess_color_black == 1);
    for(int color = 0; color < 2; ++color) // Per color.
    {
        pieces[i].color = (enum mt_chess_color)color;
        pieces[i].type = mt_chess_type_king;
        pieces[i].id = MT_CHESS_PIECE_GET_ID_BY_INDEX(i);

        ++i;
        
        for(int c = 0; c < 8; ++c)
        {
            pieces[i].color = (enum mt_chess_color)color;
            pieces[i].type = mt_chess_type_pawn;
            pieces[i].id = MT_CHESS_PIECE_GET_ID_BY_INDEX(i);
            ++i;
        }
        
        for(int c = 0; c < 2; ++c)
        {
            pieces[i].color = (enum mt_chess_color)color;
            pieces[i].type = mt_chess_type_knight;
            pieces[i].id = MT_CHESS_PIECE_GET_ID_BY_INDEX(i);
            ++i;
        }
        
        for(int c = 0; c < 2; ++c)
        {
            pieces[i].color = (enum mt_chess_color)color;
            pieces[i].type = mt_chess_type_bishop;
            pieces[i].id = MT_CHESS_PIECE_GET_ID_BY_INDEX(i);
            ++i;
        }
        
        for(int c = 0; c < 2; ++c)
        {
            pieces[i].color = (enum mt_chess_color)color;
            pieces[i].type = mt_chess_type_rook;
            pieces[i].id = MT_CHESS_PIECE_GET_ID_BY_INDEX(i);
            ++i;
        }
        
        pieces[i].color = (enum mt_chess_color)color;
        pieces[i].type = mt_chess_type_queen;
        pieces[i].id = MT_CHESS_PIECE_GET_ID_BY_INDEX(i);
        ++i;
    }
    assert(i == 2 * 2 * 8); // 32, for valid indices from 0 to 31.
}

void mt_chess_piece_invalidate(struct mt_chess_piece * const piece)
{
    assert(piece != NULL);
    
    piece->color = mt_chess_color_white; // Not invalid..
    piece->type = mt_chess_type_king; // Also not invalid..
    
    piece->id = MT_CHESS_PIECE_EMPTY; // Signalizes invalid piece.
}
