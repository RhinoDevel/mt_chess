
// Marcel Timm, RhinoDevel, 2025dec21

#ifndef MT_CHESS
#define MT_CHESS

#ifdef _WIN32
	#ifdef MT_EXPORT_CHESS
		#define MT_EXPORT_CHESS_API __declspec(dllexport)
	#else
		#define MT_EXPORT_CHESS_API __declspec(dllimport)
	#endif //MT_EXPORT_CHESS
#else //_WIN32
	#define MT_EXPORT_CHESS_API
	#ifndef __stdcall
		#define __stdcall
	#endif //__stdcall
#endif //_WIN32

#ifdef __cplusplus
    //#include <cstdint>
#else //__cplusplus
    //#include <stdint.h>
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
 */
MT_EXPORT_CHESS_API char* __stdcall mt_chess_get_board_as_str(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //MT_CHESS
