
// Marcel Timm, RhinoDevel, 2025dec21

#ifndef MT_CHESS
#define MT_CHESS

#ifdef _WIN32
	#ifdef MT_EXPORT_CHESS
		#define MT_EXPORT_CHESS_API __declspec(dllexport)
	#else
        #ifdef MT_IMPORT_CHESS
			#define MT_EXPORT_CHESS_API __declspec(dllimport)
		#else
			#define MT_EXPORT_CHESS_API
		#endif //MT_IMPORT_CHESS
	#endif //MT_EXPORT_CHESS
#else //_WIN32
	#define MT_EXPORT_CHESS_API
	#ifndef __stdcall
		#define __stdcall
	#endif //__stdcall
#endif //_WIN32

#ifdef __cplusplus
    #include <cstdbool>
#else //__cplusplus
    #include <stdbool.h>
#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

MT_EXPORT_CHESS_API void __stdcall mt_chess_free(void * const ptr);

MT_EXPORT_CHESS_API void __stdcall mt_chess_deinit(void);
MT_EXPORT_CHESS_API void __stdcall mt_chess_reinit(void);

/**
 * - Caller takes ownership of return value.
 * - Free return value with mt_chess_free() later.
 * - Returns NULL, if not initialized, yet.
 */
MT_EXPORT_CHESS_API char* __stdcall mt_chess_create_board_as_str(
	bool const unicode);

/**
 * - Caller does NOT take ownership of eventually set output message.
 */
MT_EXPORT_CHESS_API bool __stdcall mt_chess_try_move(
    char const from_file, char const from_rank,
    char const to_file, char const to_rank,
    char const * * const out_msg);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //MT_CHESS
