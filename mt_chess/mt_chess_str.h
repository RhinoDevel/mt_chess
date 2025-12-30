
// Marcel Timm, RhinoDevel, 2025dec30

#ifndef MT_CHESS_STR
#define MT_CHESS_STR

#ifdef __cplusplus
	#include <cstdbool>
#else //__cplusplus
    #include <stdbool.h>
#endif //__cplusplus

#include "mt_chess_data.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/**
 * - Caller takes ownership of return value.
 */
char* mt_chess_str_create_board(
	struct mt_chess_data const * const data, bool const unicode);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //MT_CHESS_STR
