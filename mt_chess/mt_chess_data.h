
// Marcel Timm, RhinoDevel, 2025dec21

#ifndef MT_CHESS_DATA
#define MT_CHESS_DATA

#ifdef __cplusplus
    #include <cstdint>
#else //__cplusplus
    #include <stdint.h>
#endif //__cplusplus

#include "mt_chess_piece.h"
#include "mt_chess_log_node.h"
#include "mt_chess_row.h"
#include "mt_chess_col.h"
#include "mt_chess_color.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

struct mt_chess_data
{
    struct mt_chess_piece pieces[2 * 2 * ((int)mt_chess_col_h + 1)];
    
    // Holds mt_chess_piece.id values or 0 (= empty).
    uint8_t board[((int)mt_chess_row_1 + 1) * ((int)mt_chess_col_h + 1)];
    
    // Squares being attacked by black.
    uint8_t attacked_by_black[
        ((int)mt_chess_row_1 + 1) * ((int)mt_chess_col_h + 1)];

    // Squares being attacked by white.
    uint8_t attacked_by_white[
        ((int)mt_chess_row_1 + 1) * ((int)mt_chess_col_h + 1)];

    enum mt_chess_color turn;

    struct mt_chess_log_node * log;
};

/**
 * - Will take ownership of given object (pointer will be invalid after call).
 */
void mt_chess_data_free(struct mt_chess_data * const data);

/**
 * - Free via mt_chess_data_free() after usage.
 * - Just does nothing, if NULL given.
 */
struct mt_chess_data * mt_chess_data_create(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //MT_CHESS_DATA

