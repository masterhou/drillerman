#ifndef DEFAULTS_H
#define DAFAULTS_H

#define _STR_BUFLEN 256

#define _SCREEN_WIDTH 800
#define _SCREEN_HEIGHT 600

#define _MAP_WIDTH 11
#define _BRICK_TYPE_COUNT 47

#define _BRICK_WIDTH 60
#define _BRICK_HEIGHT 60

#define _LEVEL_COUNT 1

#define _ACTION_AREA_WIDTH (_MAP_WIDTH * _BRICK_WIDTH)
#define _VISIBLE_ROWS (_SCREEN_HEIGHT / _BRICK_HEIGHT)

#define _BCG_LAYER_COUNT 3
#define _BCG_PARALLAX_FACTOR 0.25

#define _PLAYER_WIDTH 43
#define _PLAYER_HEIGHT 76

#define _PLAYER_WIDTH2 (_PLAYER_WIDTH / 2)

#define _PLAYER_SPRITE_PADDING_X 8
#define _PLAYER_SPRITE_PADDING_Y 0

#define _PLAYER_SPEED 200.0
#define _PLAYER_FALL_SPEED 200.0
#define _PLAYER_SLIP_SPEED 400.0

#define _BRICK_FALL_SPEED _PLAYER_FALL_SPEED

#define _SHAKE_MAX_SHIFT 4.0
#define _SHAKE_SPEED 70.0
#define _SHAKE_TIME 1.0

#define _MAP_OFFSET_X 0.0
#define _MAP_OFFSET_Y 300.0

/* how many bricks merged together
   causes them to explode on fall */
#define _DESTROY_SIZE_THRESHOLD 4
#define _DESTROY_BLINK_FREQUENCY 16.0
#define _BLINK_TIME_BEFORE_DESTROY 0.25

#define _BRICK_FALL_DELAY 4.0

#define _HIT_DISTANCE_THRESHOLD 5.0

#define _PARTICLE_ENGINE_CAPACITY_OVERHEAD 10
#define _PARTICLE_ENGINE_GRAVITY 400.0

#define _SPRITE_ENGINE_CAPACITY_OVERHEAD 10

#define _HIT_PARTICLE_FALL_SPEED 200.0
#define _HIT_PARTICLE_COUNT 10
#define _HIT_PARTICLE_FADE_SPEED 2.0

#define _DELAY_BEFORE_CLIMB 0.3

#define _DESTROY_PARTICLE_TYPE_COUNT 3
#define _DESTROY_PARTICLE_COUNT 10
#define _DESTROY_PARTICLE_FADE_SPEED 2.0
#define _DESTROY_PARTICLE_MAX_SPEED 200.0
#define _DESTROY_PARTICLE_MAX_ROT_SPEED 360.0

#define _AIR_DEST_X 700.0
#define _AIR_DEST_Y 200.0
#define _AIR_FLY_SPEED 800.0
#define _AIR_ROT_SPEED 720.0
#define _AIR_TRAIL_SPACING 50.0

#define _INTER_ROW_COUNT ((_SCREEN_HEIGHT / _BRICK_HEIGHT) / 2 + 1)
#define _INTER_FADE_SPEED 2.0
#define _INTER_LAYER 10
#define _INTER_BLOW_DELAY 0.2

#ifndef _DATA_PATH 
#define _DATA_PATH "./data/"
#endif

#undef __BASH_COLORS

#endif
