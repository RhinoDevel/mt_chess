
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

// Signalizes empty square on board AND invalid piece:
#define MT_CHESS_PIECE_EMPTY 0

// See mt_chess_piece_init():
#define MT_CHESS_PIECE_GET_ID_BY_INDEX(i) ((uint8_t)((i) + 1))
#define MT_CHESS_PIECE_GET_INDEX_BY_ID(id) ((int)(id) - 1)

struct mt_chess_piece // Not just an enum., e.g., because of pawn promotion.
{
    enum mt_chess_color color;
    enum mt_chess_type type;
    uint8_t id; // 0 for invalid/unset (0 also means empty cell on the board).
};

int mt_chess_piece_get_index_by_type_and_color(
    struct mt_chess_piece const * const pieces,
    enum mt_chess_type const type,
    enum mt_chess_color const color);

void mt_chess_piece_init(struct mt_chess_piece * const pieces);

void mt_chess_piece_invalidate(struct mt_chess_piece * const piece);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //MT_CHESS_PIECE
