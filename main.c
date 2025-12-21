
// Marcel Timm, RhinoDevel, 2025dec21

#include <stdio.h>

#include "mt_chess.h"

int main(void)
{
    mt_chess_reinit();
    
    char * const board_str = mt_chess_get_board_as_str();
    
    printf("%s", board_str);
    
    mt_chess_free(board_str);
    
    mt_chess_deinit();
    return 0;
}
