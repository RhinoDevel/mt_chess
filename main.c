
// Marcel Timm, RhinoDevel, 2025dec21

#ifdef __cplusplus
    #include <cstdio>
    #include <cstdbool>
    #include <cassert>
#else //__cplusplus
    #include <stdio.h>
    #include <stdbool.h>
    #include <assert.h>
#endif //__cplusplus

#include "mt_chess.h"

/**
 * - mt_chess must have been initialized via mt_chess_reinit().
 */
static void print_board()
{
    char * const board_str = mt_chess_get_board_as_str();
    
    printf("%s", board_str);
    
    mt_chess_free(board_str);
}

static bool read_pos(char * const file, char * const rank)
{
    assert(file != NULL);
    assert(rank != NULL);
    
    char pos[2 + 1 + 1];
    
    if(fgets(pos, (int)sizeof(pos), stdin) != pos)
    {
        printf("An error occurred during position retrieval!\n");
        return false;
    }
    
    if(pos[0] < 'a' || 'h' < pos[0])
    {
        printf("Position must begin with a letter (the file) between a and h.\n");
        return false;
    }
    if(pos[1] < '1' || '8' < pos[1])
    {
        printf("Second character of position must be a number (the rank) between 1 and 8.\n");
        return false;
    }
    
    if(pos[2] != '\n')
    {
        printf("Position must be two characters long (file and rank).\n");
        return false;
    }
    
    assert(pos[3] == '\0');
    
    *file = pos[0];
    *rank = pos[1];
    return true;
}



int main(void)
{
    mt_chess_reinit();
    
    do
    {
        print_board();
        
        char from_file = '\0';
        char from_rank = '\0';
        char to_file = '\0';
        char to_rank = '\0';
        
        do
        {
            printf("From: ");
            if(!read_pos(&from_file, &from_rank))
            {
                continue; // Called function logged.
            }
            printf("=> From file = %c, rank = %c.\n", from_file, from_rank);
            break;
        }while(true);
        
        do
        {
            printf("To:   ");
            if(!read_pos(&to_file, &to_rank))
            {
                continue; // Called function logged.
            }
            printf("=> To   file = %c, rank = %c.\n", to_file, to_rank);
            break;
        }while(true);
    }while(false); // TODO: Change into endless loop when ready.
    
    mt_chess_deinit();
    return 0;
}
