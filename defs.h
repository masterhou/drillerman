#ifndef DEFS_H
#define DEFS_H

#define _STR_BUFLEN 256

#define _FPS_LIMIT 60.0

#define _SCREEN_WIDTH 800
#define _SCREEN_HEIGHT 480

#define _MAP_WIDTH 11
#define _BRICK_TYPE_COUNT 47

#define _BRICK_WIDTH 45
#define _BRICK_HEIGHT 45

#define _LEVEL_COUNT 2

#define _ACTION_AREA_WIDTH (_MAP_WIDTH * _BRICK_WIDTH)
#define _VISIBLE_ROWS (_SCREEN_HEIGHT / _BRICK_HEIGHT)

#define _BCG_LAYER_COUNT 3
#define _BCG_PARALLAX_FACTOR 0.1

#define _PLAYER_WIDTH 31
#define _PLAYER_HEIGHT 52

#define _PLAYER_WIDTH2 (_PLAYER_WIDTH / 2)

#define _PLAYER_SPRITE_PADDING_X 10
#define _PLAYER_SPRITE_PADDING_Y 0

#define _PLAYER_SPEED 220.0
#define _PLAYER_FALL_SPEED 300.0
#define _PLAYER_SLIP_SPEED 200.0
#define _PLAYER_DRILL_DELAY 0.15

#define _BRICK_FALL_SPEED _PLAYER_FALL_SPEED

#define _SHAKE_MAX_SHIFT 5.0
#define _SHAKE_SPEED 150.0
#define _SHAKE_TIME 1.0


#define _INTER_ROW_COUNT 7
#define _INTER_FADE_SPEED 2.0
#define _INTER_BLOW_DELAY 0.2

#define _MAP_OFFSET_X 0.0
#define _MAP_OFFSET_Y ((_INTER_ROW_COUNT - 1) * _BRICK_HEIGHT)

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

#define _DRILL_PARTICLE_FALL_SPEED 200.0
#define _DRILL_PARTICLE_COUNT 10
#define _DRILL_PARTICLE_FADE_SPEED 2.0

#define _DELAY_BEFORE_CLIMB 0.3

#define _EXPLODE_PARTICLE_TYPE_COUNT 3
#define _EXPLODE_PARTICLE_COUNT 10
#define _EXPLODE_PARTICLE_FADE_SPEED 2.0
#define _EXPLODE_PARTICLE_MAX_SPEED 300.0
#define _EXPLODE_PARTICLE_MAX_ROT_SPEED 360.0

#define _AIR_DEST_X 700.0
#define _AIR_DEST_Y 200.0
#define _AIR_FLY_SPEED 800.0
#define _AIR_ROT_SPEED 720.0
#define _AIR_TRAIL_SPACING 50.0

#define _LEVEL_ADVANCE_FADE_SPEED 0.5
#define _LEVEL_ADVANCE_VELOCITY 250.0

#define _BRICKS_LAYER 200
#define _INTER_LAYER 210
#define _PLAYER_LAYER 220
#define _DRILL_PARTICLES_LAYER 230
#define _EXPLODE_PARTICLES_LAYER 240
#define _HUD_FONT_LAYER 10
#define _POINTS_PARTICLE_LAYER 230

#define _BRICK_SINGLE_TYPE_NR 8

#define _POINTS_PARTICLE_SPEED 200.0
#define _POINTS_PARTICLE_FADESPEED 0.8

#define _POINTS_FOR_AIR 20

#define _AIR_RESTORE 20.0
#define _AIR_DECREASE_SPEED 1.5

#ifndef _DATA_PATH 
#define _DATA_PATH "./data/"
#endif

#undef __BASH_COLORS

#endif
