
// Marcel Timm, RhinoDevel, 2025dec28

#ifndef MT_CHESS_ATTACK
#define MT_CHESS_ATTACK

#ifdef __cplusplus
    #include <cstdint>
#else //__cplusplus
    #include <stdint.h>
#endif //__cplusplus

#include "mt_chess_piece.h"
#include "mt_chess_color.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/** Update the attack map given, with the player of given color as attacker.
 */
void mt_chess_attack_update(
    struct mt_chess_piece const * const pieces,
    uint8_t const * const board,
    enum mt_chess_color const attacker,
    uint8_t * const attack_map);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //MT_CHESS_ATTACK
