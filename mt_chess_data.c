
// Marcel Timm, RhinoDevel, 2025dec21

#ifdef __cplusplus
    #include <cstdlib>
    #include <cstdint>
    #include <cassert>
    #include <cstdbool>
#else //__cplusplus
    #include <stdlib.h>
    #include <stdint.h>
    #include <assert.h>
    #include <stdbool.h>
#endif //__cplusplus

#include "mt_chess_data.h"
#include "mt_chess_piece.h"
#include "mt_chess_row.h"
#include "mt_chess_col.h"
#include "mt_chess_color.h"
#include "mt_chess_type.h"

static int const s_row_len = (int)mt_chess_col_h + 1;

/**
 * - It is wasteful using this function to get the ID of a specific piece
 *   described by the given parameters, but it is used to avoid hard-coded stuff
 *   and won't be used in performance-critical sections.
 */
static uint8_t get_piece_id(
    struct mt_chess_piece const * const pieces,
    enum mt_chess_color const color,
    enum mt_chess_type const type,
    int skip)
{
    assert(pieces != NULL);
    // Also assuming that array the pointer points to has 2 x 2 x 8 items.
    
    uint8_t ret_val = 0;
    int skipped = 0;
    
    for(int i = 0; i < 2 * 2 * 8; ++i)
    {
        if(pieces[i].color != color)
        {
            continue;
        }
        if(pieces[i].type != type)
        {
            continue;
        }
        if(skipped < skip)
        {
            ++skipped;
            continue;
        }
        ret_val = pieces[i].id;
        break; // Found
    }
    
    assert(ret_val != 0);
    return ret_val;
}

static void add_pieces_of_given_color_to_board(
    enum mt_chess_color const color,
    struct mt_chess_piece const * const pieces,
    uint8_t * const board)
{
    assert(color == mt_chess_color_black || color == mt_chess_color_white);
    assert(pieces != NULL);
    assert(board != NULL);
    // Also assumes that mt_chess_col_a to mt_chess_col_h equal 0 to 7.
    // Also assumes that mt_chess_row_8 to mt_chess_row_1 equal 0 to 7 (yes!).
    
    int row = 0;
    int col = 0;
    
    // *************************************************************************
    // *** Add officers:                                                     ***
    // *************************************************************************
    
    if(color == mt_chess_color_black)
    {
        row = (int)mt_chess_row_8;
    }
    else
    {
        assert(color == mt_chess_color_white);
        row = (int)mt_chess_row_1;
    }
    col = (int)mt_chess_col_a;
    
    board[row * s_row_len + col] = get_piece_id(
        pieces, color, mt_chess_type_rook, 0);
    ++col;
    board[row * s_row_len + col] = get_piece_id(
        pieces, color, mt_chess_type_knight, 0);
    ++col;
    board[row * s_row_len + col] = get_piece_id(
        pieces, color, mt_chess_type_bishop, 0);
    ++col;
    board[row * s_row_len + col] = get_piece_id(
        pieces, color, mt_chess_type_queen, 0);
    ++col;
    board[row * s_row_len + col] = get_piece_id(
        pieces, color, mt_chess_type_king, 0);
    ++col;
    board[row * s_row_len + col] = get_piece_id(
        pieces, color, mt_chess_type_bishop, 1);
    ++col;
    board[row * s_row_len + col] = get_piece_id(
        pieces, color, mt_chess_type_knight, 1);
    ++col;
    board[row * s_row_len + col] = get_piece_id(
        pieces, color, mt_chess_type_rook, 1);
    
    // *************************************************************************
    // *** Add pawns:                                                        ***
    // *************************************************************************
    
    if(color == mt_chess_color_black)
    {
        row = (int)mt_chess_row_7;
    }
    else
    {
        row = (int)mt_chess_row_2;
    }
    col = (int)mt_chess_col_a;
    
    while(col <= (int)mt_chess_col_h)
    {
        board[row * s_row_len + col] = get_piece_id(
            pieces, color, mt_chess_type_pawn, col);
        
        ++col;
    }
}

static void clear_empty_center_of_board(uint8_t * const board)
{
    // Also assumes that mt_chess_col_a to mt_chess_col_h equal 0 to 7.
    // Also assumes that mt_chess_row_8 to mt_chess_row_1 equal 0 to 7 (yes!)
    
    for(int row = (int)mt_chess_row_6; row <= (int)mt_chess_row_3; ++row)
    {
        int const row_offset = row * s_row_len;
        
        for(int col = (int)mt_chess_col_a; col <= (int)mt_chess_col_h; ++col)
        {
            board[row_offset + col] = 0; // (IDs of pieces must never be zero)
        }
    }
}

static void init_board(
    struct mt_chess_piece const * const pieces, uint8_t * const board)
{   
    add_pieces_of_given_color_to_board(mt_chess_color_black, pieces, board);
    
    clear_empty_center_of_board(board);
    
    add_pieces_of_given_color_to_board(mt_chess_color_white, pieces, board);
}

void mt_chess_data_free(struct mt_chess_data * const data)
{
    if(data == NULL)
    {
        assert(false); // Although no problem (here).
        return;
    }
    
    if(data->log != NULL)
    {
        mt_chess_log_node_free(data->log);
        data->log = NULL;
    }
    free(data);
}

struct mt_chess_data * mt_chess_data_create()
{
    struct mt_chess_data * const ret_val =
        (struct mt_chess_data *)malloc(sizeof *ret_val);
        
    assert(ret_val != NULL);
    
    mt_chess_piece_init(ret_val->pieces);
    init_board(ret_val->pieces, ret_val->board);
    ret_val->log = NULL;
    
    return ret_val;
}