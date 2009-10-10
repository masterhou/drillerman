#include "hud.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "defs.h"
#include "snge.h"
#include "sprites.h"

static Sprite *sPoints;
static Sprite *sDepth;
static Sprite *sPointsLabel;
static Sprite *sDepthLabel;

static Sprite *airGauge;

static void setTextFromInt(Sprite *fontSprite, int value)
{
    snprintf(fontSprite->text, _STR_BUFLEN, "%d", value);
}

void hud_Init()
{
    SpriteClassId mainFont = sprites_GetIdByName("font:base");
    SpriteClassId labelFont = sprites_GetIdByName("font:hollow");
    SpriteClassId smallFont = sprites_GetIdByName("font:small");

    int hx = _ACTION_AREA_WIDTH + 20;\
    int hy = 20;

    sPointsLabel = snge_AddFontSprite(labelFont, point(hx, hy), _HUD_FONT_LAYER, "Points");
    sPointsLabel->relative = true;

    sPoints = snge_AddFontSprite(mainFont, point(hx, hy + 40), _HUD_FONT_LAYER, "0");
    sPoints->relative = true;

    sDepthLabel = snge_AddFontSprite(labelFont, point(hx, hy + 102), _HUD_FONT_LAYER, "DEPTH");
    sDepthLabel->relative = true;

    sDepth = snge_AddFontSprite(mainFont, point(hx, hy + 142), _HUD_FONT_LAYER, "0");
    sDepth->relative = true;

    airGauge = snge_AddGaugeSprite(sprites_GetIdByName("level_common:gauge-fill"), point(hx, hy + 260), _HUD_FONT_LAYER, GAUGE_DU);
    airGauge->gaugeFill = 1.0;
    airGauge->relative = true;

    Sprite *s = snge_AddSprite(sprites_GetIdByName("level_common:gauge-over"), point(hx, hy + 260), _HUD_FONT_LAYER + 1);
    s->relative = true;
}

void hud_SetPoints(int points)
{
    setTextFromInt(sPoints, points);
}

void hud_SetDepth(int depth)
{
    setTextFromInt(sDepth, depth);
}

void hud_SetAir(float percent)
{
    airGauge->gaugeFill = percent / 100.0;
}




