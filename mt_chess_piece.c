
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

int mt_chess_piece_get_index(struct mt_chess_piece * const pieces, int const id)
{
    assert(pieces != NULL);
    // Also assuming that array the pointer points to has 2 x 2 x 8 items.
    
    assert(1 <= id && id <= 32); // Although no problem, here.
    
    for(int i = 0; i < 2 * 2 * 8; ++i)
    {
        if(pieces[i].id == id)
        {
            return i;
        }
    }
    assert(false);
    return -1; // Not found.
}

void mt_chess_piece_init(struct mt_chess_piece * const pieces)
{
    assert(pieces != NULL);
    // Also assuming that array the pointer points to has 2 x 2 x 8 items.
    
    int i = 0; // Per piece.
    uint8_t id = 1; // Don't start with 0, 0 means empty cell (elsewhere).
    
    assert((int)mt_chess_color_white == 0);
    assert((int)mt_chess_color_black == 1);
    for(int color = 0; color < 2; ++color) // Per color.
    {
        pieces[i].color = (enum mt_chess_color)color;
        pieces[i].type = mt_chess_type_king;
        pieces[i].id = id;
        ++i;
        ++id;
        
        for(int c = 0; c < 8; ++c)
        {
            pieces[i].color = (enum mt_chess_color)color;
            pieces[i].type = mt_chess_type_pawn;
            pieces[i].id = id;
            ++i;
            ++id;
        }
        
        for(int c = 0; c < 2; ++c)
        {
            pieces[i].color = (enum mt_chess_color)color;
            pieces[i].type = mt_chess_type_knight;
            pieces[i].id = id;
            ++i;
            ++id;
        }
        
        for(int c = 0; c < 2; ++c)
        {
            pieces[i].color = (enum mt_chess_color)color;
            pieces[i].type = mt_chess_type_bishop;
            pieces[i].id = id;
            ++i;
            ++id;
        }
        
        for(int c = 0; c < 2; ++c)
        {
            pieces[i].color = (enum mt_chess_color)color;
            pieces[i].type = mt_chess_type_rook;
            pieces[i].id = id;
            ++i;
            ++id;
        }
        
        pieces[i].color = (enum mt_chess_color)color;
        pieces[i].type = mt_chess_type_queen;
        pieces[i].id = id;
        ++i;
        ++id;
    }
    assert(i == 2 * 2 * 8); // 32, for valid indices from 0 to 31.
    assert(id == i + 1); // 33, for valid IDs from 1 to 32.
}

void mt_chess_piece_invalidate(struct mt_chess_piece * const piece)
{
    assert(piece != NULL);
    
    piece->color = mt_chess_color_white; // Not invalid..
    piece->type = mt_chess_type_king; // Also not invalid..
    
    piece->id = 0; // Signalizes invalid piece.
}
