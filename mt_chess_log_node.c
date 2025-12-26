
// Marcel Timm, RhinoDevel, 2025dec21

#ifdef __cplusplus
    #include <cstdlib>
    #include <cassert>
    #include <cstdbool>
#else //__cplusplus
    #include <stdlib.h>
    #include <assert.h>
    #include <stdbool.h>
#endif //__cplusplus

#include "mt_chess_log_node.h"
#include "mt_chess_move.h"

void mt_chess_log_node_free(struct mt_chess_log_node * const node)
{
    if(node == NULL)
    {
        assert(false); // Although no problem (here).
        return;
    }
    
    //assert(node->last == NULL); // Although no problem (here).
    
    struct mt_chess_log_node * const next = node->next;
    
    free(node);
    if(next != NULL)
    {
        mt_chess_log_node_free(next); // *** RECURSION ***
    }
}

struct mt_chess_log_node * mt_chess_log_node_create()
{
    struct mt_chess_log_node * const ret_val =
        (struct mt_chess_log_node *)malloc(sizeof *ret_val);
        
    assert(ret_val != NULL);
    
    ret_val->last = NULL;
    mt_chess_move_invalidate(&ret_val->move);
    ret_val->next = NULL;
    
    return ret_val;
}

struct mt_chess_log_node * mt_chess_log_node_get_latest(
    struct mt_chess_log_node * const node)
{
    struct mt_chess_log_node * ret_val = node;

    if(ret_val == NULL)
    {
        return NULL/*ret_val*/;
    }

    while(ret_val->next != NULL)
    {
        ret_val = ret_val->next;
    }
    return ret_val;
}
