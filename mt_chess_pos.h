
// Marcel Timm, RhinoDevel, 2025dec21

#ifndef MT_CHESS_POS
#define MT_CHESS_POS

#ifdef __cplusplus
    #include <cstdint>
#else //__cplusplus
    #include <stdint.h>
#endif //__cplusplus

struct mt_chess_pos
{
    uint8_t col; // Holds value of enumeration mt_chess_col.
    uint8_t row; // Holds value of enumeration mt_chess_row.
};

void mt_chess_pos_invalidate(struct mt_chess_pos * const pos);

#endif //MT_CHESS_POS
