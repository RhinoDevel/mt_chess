
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

#endif //MT_CHESS_LOG_NODE
