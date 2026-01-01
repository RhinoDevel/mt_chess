
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

// Library is neither exported, nor imported. Code is directly compiled-in:
//#define MT_EXPORT_CHESS
//#define MT_IMPORT_CHESS

#include "mt_chess.h"

#ifdef _WIN32
    #include <windows.h> // For SetConsoleOutputCP().
#endif //_WIN32

static void clear_console_and_to_top_left(void)
{
    printf("\033[H\033[J");
}

/**
 * - mt_chess must have been initialized via mt_chess_reinit().
 */
static void print_board(void)
{
    char * const board_str = mt_chess_create_board_as_str(true);
    
    printf("%s", board_str);
    
    mt_chess_free(board_str);
}

static bool read_pos(char * const out_file, char * const out_rank)
{
    assert(out_file != NULL);
    assert(out_rank != NULL);
    
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
    
    *out_file = pos[0];
    *out_rank = pos[1];
    return true;
}

int main(void)
{
    bool new_game = true;
    char const * move_msg = NULL;
    bool move_succeeded = false;

    char from_file = '\0';
    char from_rank = '\0';
    char to_file = '\0';
    char to_rank = '\0';

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif //_WIN32

    mt_chess_reinit();
    
    do
    {
        clear_console_and_to_top_left();
        if(new_game)
        {
            assert(move_msg == NULL);
            assert(!move_succeeded);
            printf("New game.\n");
            new_game = false;
        }
        else
        {
            if(move_succeeded)
            {
                assert(move_msg == NULL);
                printf(
                    "Last move: %c%c to %c%c\n",
                    from_file, from_rank, to_file, to_rank);
            }
            else
            {
                assert(move_msg != NULL);
                printf(
                    "%c%c to %c%c failed: \"%s\"\n",
                    from_file, from_rank, to_file, to_rank,
                    move_msg);
                move_msg = NULL;
            }
            move_succeeded = false;
        }
        print_board();

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
        
        if(!mt_chess_try_move(
                from_file, from_rank, to_file, to_rank, &move_msg))
        {
            assert(move_msg != NULL);
            assert(!move_succeeded);
            continue;
        }
        assert(move_msg == NULL);
        move_succeeded = true;
    }while(true);
    
    mt_chess_deinit();
    return 0;
}
