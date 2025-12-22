
// Marcel Timm, RhinoDevel, 2025dec21

#ifdef __cplusplus
    #include <cstdint>
    #include <cassert>
    #include <cstdbool>
    #include <cstdlib>
#else //__cplusplus
    #include <stdint.h>
    #include <assert.h>
    #include <stdbool.h>
    #include <stdlib.h>
#endif //__cplusplus

#include "mt_chess_data.h"
#include "mt_chess_piece.h"
#include "mt_chess_color.h"
#include "mt_chess_type.h"
#include "mt_chess.h"

static struct mt_chess_data * s_data = NULL;

MT_EXPORT_CHESS_API void __stdcall mt_chess_free(void * const ptr)
{
    if(ptr == NULL)
    {
        assert(false); // Although no problem, here.
        return;
    }
    free(ptr);
}

MT_EXPORT_CHESS_API void __stdcall mt_chess_deinit(void)
{
    if(s_data == NULL)
    {
        assert(false); // Although no problem.
        return;
    }
    mt_chess_data_free(s_data);
    s_data = NULL;
}

MT_EXPORT_CHESS_API void __stdcall mt_chess_reinit(void)
{
    if(s_data != NULL)
    {
        mt_chess_deinit();
    }
    assert(s_data == NULL);
    s_data = mt_chess_data_create();
    assert(s_data != NULL);
}

MT_EXPORT_CHESS_API char* __stdcall mt_chess_get_board_as_str(void)
{
    if(s_data == NULL)
    {
        // Not initialized, yet.
        assert(false);
        return NULL;
    }
    
    //                                 
    // 0000000000111111111122222222223333333333444444444455
    // 0123456789012345678901234567890123456789012345678901
    //   -------------------------------------------------\n    0
    // 8 | b_r | b_n | b_b | b_q | b_k | b_b | b_n | b_r |\n    1  1/2 = 0; 8 - 0 = 8
    //   -------------------------------------------------\n    2 
    // 7 | b_p | b_p | b_p | b_p | b_p | b_p | b_p | b_p |\n    3  3/2 = 1; 8 - 1 = 7
    //   -------------------------------------------------\n    4
    // 6 |#####|     |     |#####|     |     |#####|     |\n    5  5/2 = 2; 8 - 2 = 6
    //   -------------------------------------------------\n    6
    // 5 |     |#####|     |#####|     |#####|     |#####|\n    7  7/2 = 3; 8 - 3 = 5
    //   -------------------------------------------------\n    8
    // 4 |#####|     |     |#####|     |     |#####|     |\n    9  9/2 = 4; 8 - 4 = 4
    //   -------------------------------------------------\n   10
    // 3 |     |#####|     |#####|     |#####|     |#####|\n   11 11/2 = 5; 8 - 5 = 3
    //   -------------------------------------------------\n   12
    // 2 | w_p | w_p | w_p | w_p | w_p | w_p | w_p | w_p |\n   13 13/2 = 6; 8 - 6 = 2
    //   ------------------------------------------------|\n   14 
    // 1 | w_r | w_n | w_b | w_q | w_k | w_b | w_n | w_r |\n   15 15/2 = 7; 8 - 7 = 1
    //   ------------------------------------------------|\n   16
    //      a     b     c     d     e     f     g     h   \n\0 17
    
    static int const rows = 18;
    static int const columns = 51 + 1; // Incl. newlines.
    static int const chars = rows * columns + 1; // Incl. 0-termination.
    char * const ret_val = (char*)malloc((size_t)chars * sizeof *ret_val);
    
    assert(ret_val != NULL);
    
    for(int row = 0; row < rows; ++row)
    {
        int const row_offset = row * columns;
        bool const is_sep = row % 2 == 0;
        
        for(int col = 0; col < columns; ++col)
        {
            int const col_offset = row_offset + col;
            
            if(is_sep)
            {
                if(col < 2)
                {
                    ret_val[col_offset] = ' ';
                    continue;
                }
                if(col + 1 < columns)
                {
                    ret_val[col_offset] = '-';
                    continue;
                }
                ret_val[col_offset] = '\n';
                continue;
            }
            
            if(row + 1 == rows)
            {
                int const buf = col - 5;
                
                if(buf % 6 == 0)
                {
                    ret_val[col_offset] = (char)((int)'a' + buf / 6);
                    continue;
                }
                if(col + 1 < columns)
                {
                    ret_val[col_offset] = ' ';
                    continue;
                }
                ret_val[col_offset] = '\n';
                continue;
            }
            
            if(col == 0)
            {
                int const rank = 8 - row / 2;
                
                if(0 < rank)
                {
                    ret_val[col_offset] = (char)((int)'0' + rank);
                    continue;
                }
                ret_val[col_offset] = ' ';
                continue;
            }
            if(col == 1)
            {
                ret_val[col_offset] = ' ';
                continue;
            }
            if(col == 2)
            {
                ret_val[col_offset] = '|';
                continue;
            }
            
            if(3 <= col && col < columns - 2)
            {
                if((col - 2) % 6 == 0)
                {
                    ret_val[col_offset] = '|';
                    continue;
                }
            
                int const buf = col - 3;
                
                if(buf % 6 == 0)
                {
                    int const board_col = buf / 6;
                    int const board_row = row / 2;
                    int const board_index =
                            board_row * ((int)mt_chess_col_h + 1) + board_col;
                            
                    assert(board_index < 8 * 8);
                    
                    uint8_t const piece_id = s_data->board[board_index];
                    
                    if(piece_id == 0)
                    {
                        // Empty.
                        
                        if(board_row % 2 == board_col % 2)
                        {
                            // White cell.
                            
                            ret_val[col_offset] = '#';
                            ret_val[col_offset + 1] = '#';
                            ret_val[col_offset + 2] = '#';
                            ret_val[col_offset + 3] = '#';
                            ret_val[col_offset + 4] = '#';
                        }
                        else
                        {
                            // Black cell.
                            
                            ret_val[col_offset] = ' ';
                            ret_val[col_offset + 1] = ' ';
                            ret_val[col_offset + 2] = ' ';
                            ret_val[col_offset + 3] = ' ';
                            ret_val[col_offset + 4] = ' ';
                        }
                    }
                    else
                    {
                        // NOT empty.
                        
                        ret_val[col_offset] = ' ';
                        
                        int const piece_index = mt_chess_piece_get_index(
                                s_data->pieces, piece_id);
                        
                        assert(0 <= piece_index && piece_index < 2 * 2 * 8);
                        
                        if(s_data->pieces[piece_index].color
                            == mt_chess_color_black)
                        {
                            ret_val[col_offset + 1] = 'b'; // Black piece.
                        }
                        else
                        {
                            assert(s_data->pieces[piece_index].color
                                == mt_chess_color_white);
                            
                            ret_val[col_offset + 1] = 'w'; // White piece.
                        }
                        
                        ret_val[col_offset + 2] = '_';
                        
                        switch(s_data->pieces[piece_index].type)
                        {
                            case mt_chess_type_king:
                            {
                                ret_val[col_offset + 3] = 'k'; // King
                                break;
                            }
                            case mt_chess_type_pawn:
                            {
                                ret_val[col_offset + 3] = 'p'; // Pawn
                                break;
                            }
                            case mt_chess_type_knight:
                            {
                                ret_val[col_offset + 3] = 'n'; // Knight
                                break;
                            }
                            case mt_chess_type_bishop:
                            {
                                ret_val[col_offset + 3] = 'b'; // Bishop
                                break;
                            }
                            case mt_chess_type_rook:
                            {
                                ret_val[col_offset + 3] = 'r'; // Rook
                                break;
                            }
                            case mt_chess_type_queen:
                            {
                                ret_val[col_offset + 3] = 'q'; // Queen
                                break;
                            }
                            
                            default:
                            {
                                assert(false);
                                ret_val[col_offset + 3] = '?'; // Error!
                                break;
                            }
                        }
                        
                        ret_val[col_offset + 4] = ' ';
                    }
                    col += 4; // Kind of a hack..
                    continue;
                }
                
                ret_val[col_offset] = '?';
                continue;
            }
            
            if(col == columns - 2)
            {
                ret_val[col_offset] = '|';
                continue;
            }
            ret_val[col_offset] = '\n';
        }
    }
    
    ret_val[chars - 1] = '\0';
    
    return ret_val;
}

MT_EXPORT_CHESS_API bool __stdcall mt_chess_try_move(
    char const from_file, char const from_rank,
    char const to_file, char const to_rank,
    char const * * const out_msg)
{
    assert(out_msg != NULL);
    
    struct mt_chess_pos const from = mt_chess_pos_get(from_file, from_rank);
    
    if(mt_chess_pos_is_invalid(&from))
    {
        assert(false); // Should have already been handled by caller.
        *out_msg = "The given from-position is invalid.";
        return false;
    }
    
    struct mt_chess_pos const to = mt_chess_pos_get(to_file, to_rank);
    
    if(mt_chess_pos_is_invalid(&to))
    {
        assert(false); // Should have already been handled by caller.
        *out_msg = "The given to-position is invalid.";
        return false;
    }
    
    // TODO: Implement!
    *out_msg = "Not implemented!";
    return false;
}