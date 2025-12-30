
// Marcel Timm, RhinoDevel, 2025dec21

#ifndef MT_CHESS_POS
#define MT_CHESS_POS

#ifdef __cplusplus
    #include <cstdint>
    #include <cstdbool>
#else //__cplusplus
    #include <stdint.h>
    #include <stdbool.h>
#endif //__cplusplus

struct mt_chess_pos
{
    uint8_t col; // Holds value of enumeration mt_chess_col.
    uint8_t row; // Holds value of enumeration mt_chess_row.
};

void mt_chess_pos_invalidate(struct mt_chess_pos * const pos);

bool mt_chess_pos_is_invalid(struct mt_chess_pos const * const pos);

bool mt_chess_pos_are_equal(
    struct mt_chess_pos const * const a, struct mt_chess_pos const * const b);

struct mt_chess_pos mt_chess_pos_get(char const file, char const rank);

#endif //MT_CHESS_POS
