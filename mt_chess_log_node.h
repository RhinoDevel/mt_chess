
// Marcel Timm, RhinoDevel, 2025dec21

#ifndef MT_CHESS_LOG_NODE
#define MT_CHESS_LOG_NODE

#ifdef __cplusplus
    #include <cstdint>
#else //__cplusplus
    #include <stdint.h>
#endif //__cplusplus

#include "mt_chess_move.h"

struct mt_chess_log_node
{
    struct mt_chess_log_node * last;
    
    struct mt_chess_move move;
    
    struct mt_chess_log_node * next;
};

/**
 * - Will take ownership of given object (pointer will be invalid after call)
 *   and all following(!) nodes.
 * - Does NOT care about eventually referenced last node.
 * - (Currently) recursive.
 */
void mt_chess_log_node_free(struct mt_chess_log_node * const node);

/**
 * - Caller takes ownership of returned node.
 * - Return value must be freed after use with mt_chess_log_node_free().
 */
struct mt_chess_log_node * mt_chess_log_node_create();

/**
 * - Returns NULL, if NULL given.
 */
struct mt_chess_log_node * mt_chess_log_node_get_latest(
    struct mt_chess_log_node * const node);

/**
 * - Returns NULL, if NULL given.
 * - Returns NULL, if piece with given ID did not move, yet.
 */
struct mt_chess_log_node * mt_chess_log_node_get_latest_of_piece(
    struct mt_chess_log_node * const node, uint8_t const piece_id);


#endif //MT_CHESS_LOG_NODE
