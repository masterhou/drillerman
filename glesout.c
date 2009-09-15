#include "glesout.h"
#include "graphics.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <math.h>

#include "message.h"

static int extGL_ARB_NPOT;
static int extGL_ARB_TEX_RECT;

extern int filteringType;

static FillType bgType = FT_FLAT;
static GradientType bgGradient;
static Color bgColor1;
static Color bgColor2;
static Color fadeColor;

    float pfIdentity[] =
    {
        1.0f,0.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,0.0f,
        0.0f,0.0f,1.0f,0.0f,
        0.0f,0.0f,0.0f,1.0f
    };

    const char* pszVertShader = "\
                                attribute highp vec4	myVertex;\
                                uniform mediump mat4	myPMVMatrix;\
                                void main(void)\
                                {\
                                 gl_Position = myPMVMatrix * myVertex;\
                                           }";


extern int screenWidth;
extern int screenHeight;

static int queryExtenstion(char *extName)
{
    char *p = (char*)glGetString(GL_EXTENSIONS);
    char *end;
    int extNameLen;

    extNameLen = strlen(extName);
    end = p + strlen(p);

    while (p < end)
    {
        int n = strcspn(p, " ");
        if ((extNameLen == n) && (strncmp(extName, p, n) == 0))
        {
            message_OutEx("Found extenstion '%s'.\n", extName);
            return 1;
        }

        p += (n + 1);
    }

    return 0;
}


void glesoutInitSubsystem(int screen_width, int screen_height)
{

    GLuint uiVertShader;
    GLuint uiProgramObject;

    uiVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(uiVertShader, 1, (const char**)&pszVertShader, NULL);
    glCompileShader(uiVertShader);

    uiProgramObject = glCreateProgram();
    glAttachShader(uiProgramObject, uiVertShader);
    glBindAttribLocation(uiProgramObject, 0, "myVertex");
    glLinkProgram(uiProgramObject);
    glUseProgram(uiProgramObject);

    extGL_ARB_NPOT = queryExtenstion("GL_ARB_texture_non_power_of_two");
    extGL_ARB_TEX_RECT = queryExtenstion("GL_ARB_texture_rectangle");

    if(!extGL_ARB_NPOT && extGL_ARB_TEX_RECT)
    {
        message_Warning("Extension 'GL_ARB_texture_non_power_of_two' not found.\n");
        message_Warning("Using 'GL_ARB_texture_rectangle' instead.\n");
    }

    if(!extGL_ARB_NPOT && !extGL_ARB_TEX_RECT)
        message_CriticalError("No extensions allowing non-power of two textures found.\nGo buy a new graphics card.\n");

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);

    if(!extGL_ARB_NPOT)
        glEnable(GL_TEXTURE_RECTANGLE_ARB);

    glEnable(GL_VERTEX_ARRAY);
    glEnable(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_COORD_ARRAY);

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    glOrtho(0, screen_width, screen_height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glPushMatrix();

    glDisable(GL_LIGHTING);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

}

void glesoutSetBackground(FillType bt, Color main_color, Color aux_color, GradientType gt)
{
    bgType = bt;
    bgGradient = gt;
    bgColor1 = main_color;
    bgColor2 = aux_color;

    if(bt == FT_FLAT)
        glClearColor(bgColor1.r, bgColor1.g, bgColor1.b, bgColor1.a);
}

void glesoutClearBuffer()
{
    Point screenLTCorner = {0, 0};
    Point screenBRCorner = {_SCREEN_WIDTH, _SCREEN_HEIGHT};

    glClear(GL_COLOR_BUFFER_BIT);
    if(bgType == FT_GRADIENT)
        glesoutDrawRectangle(screenLTCorner, screenBRCorner, FT_GRADIENT, bgGradient, bgColor1, bgColor2);

    glLoadIdentity();
}

void glesoutBlitBuffer()
{
    Point screenLTCorner = {0, 0};
    Point screenBRCorner = {_SCREEN_WIDTH, _SCREEN_HEIGHT};

    if(fadeColor.a != 0.0)
        glesoutDrawRectangle(screenLTCorner, screenBRCorner, FT_FLAT, GT_NONE, fadeColor, bgColor2);

    SDL_GL_SwapBuffers();
}

void glesoutSetFadeColor(Color fcolor)
{
    fadeColor = fcolor;
}

BitmapId glesoutLoadBitmap(const char* file, int *w, int *h)
{

    SDL_Surface* surface = IMG_Load(file);
    GLuint texture = 0;

    if(!surface)
    {
        message_CriticalErrorEx("SDL Error: '%s'\n", SDL_GetError());
        return 0;
    }

    *w = surface->w;
    *h = surface->h;

    GLenum target;

    if(extGL_ARB_NPOT)
        target = GL_TEXTURE_2D;
    else
        target = GL_TEXTURE_RECTANGLE_ARB;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texture);

    glBindTexture(target, texture);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameterf(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    SDL_PixelFormat *format = surface->format;

    if (format->Amask)
    {
        glTexImage2D(target, 0, 4, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    }
    else
    {
        glTexImage2D(target, 0, 3, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
    }

    SDL_FreeSurface(surface);
    return texture;
}

void glesoutDrawRectangle(Point left_top, Point right_bottom, FillType ft, GradientType gt, Color main_color, Color aux_color)
{
    GLfloat vertices[8] = {
        left_top.x, left_top.y,
        right_bottom.x, left_top.y,
        right_bottom.x, right_bottom.y,
        left_top.x, right_bottom.y
    };

    GLuint indices[4] = {0, 1, 2, 3};
    GLfloat colors[16];

    if(ft == FT_FLAT)
    {
        int i;
        for(i = 0; i < 4; ++i)
        {
            colors[i * 4 + 0] = main_color.r;
            colors[i * 4 + 1] = main_color.g;
            colors[i * 4 + 2] = main_color.b;
            colors[i * 4 + 3] = main_color.a;
        }
    }

    if(ft == FT_GRADIENT)
    {
        switch(gt)
        {
                        case GT_V:

            memcpy(colors, &main_color, sizeof(float) * 4);
            memcpy(colors + 4, &main_color, sizeof(float) * 4);
            memcpy(colors + 8, &aux_color, sizeof(float) * 4);
            memcpy(colors + 12, &aux_color, sizeof(float) * 4);
            break;

                        case GT_H:

            memcpy(colors, &main_color, sizeof(float) * 4);
            memcpy(colors + 4, &aux_color, sizeof(float) * 4);
            memcpy(colors + 8, &aux_color, sizeof(float) * 4);
            memcpy(colors + 12, &main_color, sizeof(float) * 4);

            break;

                        default:;
                        }
    }

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glColorPointer(4, GL_FLOAT, 0, colors);

    if(extGL_ARB_NPOT)
        glBindTexture(GL_TEXTURE_2D, 0);
    else
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

    glDisable(GL_TEXTURE_COORD_ARRAY);
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, indices);
    glEnable(GL_TEXTURE_COORD_ARRAY);

}

void glesoutBlitBitmap(BitmapId textureID, Transformations *transfs)
{
    Point lt = {0, 0};
    Point rb = {transfs->size.x, transfs->size.y};
    glesoutBlitPartBitmap(textureID, transfs, lt, rb);

}

void glesoutBlitPartBitmap(BitmapId textureID, Transformations *transfs, Point left_top, Point part_size)
{
    extern float xScrRatio;
    extern float yScrRatio;

    if(xScrRatio != 1.0)
    {
        transfs->trans.x *= xScrRatio;
        transfs->scale.x *= xScrRatio;
        transfs->size.x *= xScrRatio;
        part_size.x *= xScrRatio;
    }

    if(yScrRatio != 1.0)
    {
        transfs->trans.y *= yScrRatio;
        transfs->scale.y *= yScrRatio;
        part_size.y *= yScrRatio;
    }

    float hw = part_size.x * transfs->scale.x / 2.0;
    float hh = part_size.y * transfs->scale.y / 2.0;

    transfs->angle %= 360;

    GLfloat vertices[8] = {
        -hw, -hh,
        hw, -hh,
        hw, hh,
        -hw, hh
    };

    GLuint indices[4] = {0, 1, 2, 3};
    GLfloat colors[16] = {
        1, 1, 1, transfs->opacity,
        1, 1, 1, transfs->opacity,
        1, 1, 1, transfs->opacity,
        1, 1, 1, transfs->opacity
    };

    float mx = transfs->size.x - 1;
    float my = transfs->size.y - 1;
    Point lt = {left_top.x / mx, left_top.y / my};
    Point rb = {(left_top.x + part_size.x - 1) / mx, (left_top.y + part_size.y - 1) / my};

    GLfloat uvsf[8] = {
        lt.x, lt.y,
        rb.x, lt.y,
        rb.x, rb.y,
        lt.x, rb.y
    };
    GLint uvsi[8] = {
        (int)left_top.x, (int)left_top.y,
        (int)left_top.x + (int)part_size.x - 1, (int)left_top.y,
        (int)left_top.x + (int)part_size.x - 1, (int)left_top.y + (int)part_size.y - 1,
        (int)left_top.x, (int)left_top.y + (int)part_size.y - 1
    };



    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glColorPointer(4, GL_FLOAT, 0, colors);

    glPushMatrix();

    glTranslatef(transfs->trans.x, transfs->trans.y, 0);

    // position by upper-left corner
    glTranslatef(hw, hh, 0);

    // rotate by center
    glRotatef(transfs->angle, 0.0, 0.0, 1.0f);

    // flip if needed
    if(transfs->vflip)
        glRotatef(180.0, 1.0f, 0.0, 0.0);

    if(transfs->hflip)
        glRotatef(180.0, 0.0, 1.0f, 0.0);

    if(extGL_ARB_NPOT)
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexCoordPointer(2, GL_FLOAT, 0, uvsf);
    }
    else
    {
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, textureID);
        glTexCoordPointer(2, GL_INT, 0, uvsi);

    }

    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, indices);

    glPopMatrix();
}

void glesoutBlitText(BitmapId texture_id, Transformations *transfs, const char *text, Font font)
{
    int rc, c, i;
    char *f;
    int len = strlen(text);

    Point r;
    Point sz = {font.char_width, font.char_height};
    Point h = {
        (float)(font.char_width * len) / 2.0,
        (float)font.char_height / 2.0
    };

    Transformations t = *transfs;

    t.angle = 0;

    glPushMatrix();

    glTranslatef(transfs->trans.x, transfs->trans.y, 0);

    glTranslatef(h.x, h.y, 0);

    glRotatef(transfs->angle, 0.0, 0.0, 1.0f);

    for(c = 0; c < len; ++c)
    {
        if(text[c] == ' ') continue;

        f = strchr(font.char_string, tolower(text[c]));

        if(!f) continue;

        i = f - font.char_string;

        Point o = {i * font.char_width, 0};

        rc = c;

        if(transfs->hflip)
            rc = len - c - 1;

        r.x = (float)(rc * font.char_width) - h.x;
        r.y = (float)(0) - h.y;

        r.x *= t.scale.x;
        r.y *= t.scale.y;

        t.trans = r;

        glesoutBlitPartBitmap(texture_id, &t, o, sz);
    }

    glPopMatrix();

}
