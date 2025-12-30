
// Marcel Timm, RhinoDevel, 2025dec21

#ifndef MT_CHESS_PIECE
#define MT_CHESS_PIECE

#ifdef __cplusplus
    #include <cstdint>
#else //__cplusplus
    #include <stdint.h>
#endif //__cplusplus

#include "mt_chess_color.h"
#include "mt_chess_type.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

struct mt_chess_piece // Not just an enum., e.g., because of pawn promotion.
{
    enum mt_chess_color color;
    enum mt_chess_type type;
    uint8_t id; // 0 for invalid/unset (0 also means empty cell on the board).
};

int mt_chess_piece_get_index(
        struct mt_chess_piece const * const pieces, uint8_t const id);

void mt_chess_piece_init(struct mt_chess_piece * const pieces);

void mt_chess_piece_invalidate(struct mt_chess_piece * const piece);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //MT_CHESS_PIECE
