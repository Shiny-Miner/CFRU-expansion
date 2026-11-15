#pragma once

#define FOLLOWER_EMOTE_THINKING 0x80
#define FOLLOWER_EMOTE_HEART    0x81
#define FOLLOWER_EMOTE_SAD      0x82
#define FOLLOWER_EMOTE_FIRST    FOLLOWER_EMOTE_THINKING
#define FOLLOWER_EMOTE_COUNT    3

void CreateFollowerEmote(u8 emote);
