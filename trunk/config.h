#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

/*
    Very simple and naive text config file parser.
    Config files cosist of sections which should be
    defined like that:

    [section]

    Properties in section should be defined:

    property_name=value

    with no whitespace between property name and the
    equal sign.

    You can use tags:

    +tag_name

    Tag, if found in a section returns true,
    otherwise returns false.

    Valid values for bool properties:
    1, 0, true, false, t, f, yes, no, y, n
*/

bool cfg_Open(const char *filepath);
void cfg_Close();

bool cfg_SeekSection(char *section_name);
bool cfg_NextSection(char *section_name);

bool cfg_GetIntValue(char *value_name, int *value);
bool cfg_GetDoubleValue(char *value_name, double *value);
bool cfg_GetStringValue(char *value_name, char *str);
bool cfg_GetTag(char *tag_name);

#endif

