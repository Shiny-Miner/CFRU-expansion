#pragma once

/*
 * MYSTERY GIFT - gift codes typed on an on-screen keyboard.
 *
 *  callasm MysteryGift_Start      (or special 0x122, after running tools/install_special.py)
 *  waitstate
 *
 * Result in LASTRESULT: 1 = a gift was received, 0 = left without receiving anything.
 * Uses variables 0x8004, 0x8005 and 0x8006 as scratch space.
 */
void MysteryGift_Start(void);
