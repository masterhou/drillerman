#ifndef GENERATOR_H
#define GENERATOR_H

#include "defaults.h"
#include "common.h"

typedef enum 
{
    VF_BRICK_RED,
    VF_BRICK_YELLOW,
    VF_BRICK_GREEN,
    VF_BRICK_BLUE,
    VF_BRICK_COUNT,
    VF_AIR,
    VF_CRATE,
    VF_NONE,
    VF_LAST,
    VF_VOID
} FieldType;

#define VF_DOES_MERGE(FTYPE) ((FTYPE) < VF_BRICK_COUNT || (FTYPE == VF_CRATE))

void generator_AllocMap(FieldType ***pmap, int height, int level);
void generator_FreeMap(FieldType ***map);


#endif
