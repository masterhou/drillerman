#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "defs.h"

static FILE* cfgfile = NULL;
static long section_offset = 0;

static void separateValue(char* str);
static bool checkPattern(char* pattern, char* str);

static void trimString(char *str)
{
    register char *start = str;
    register char *end = str + strlen(str) - 1;

    while(*start == ' ') start++;

    while((*end == '\n' || *end == ' ' || *end == '\r') && (end != start)) end--;

    unsigned int sz = end - start + 1;

    memmove(str, start, sz);

    str[sz] = '\0';
}

static bool checkPattern(char *pattern, char *str)
{
    register char *cs = str;
    register char *cp = pattern;

    while(*cs == ' ') cs++;

    while(*cs != '\0' && *cp != '\0' && *cp == *cs) cp++,cs++;

    if((cp - pattern) == strlen(pattern))
        return true;

    return false;
}

static void separateValue(char *str)
{
    register char *cs = str;

    while(*cs != '=' && *cs != '\0') cs++;

    if(*cs == '\0')
    {
        message_WarningEx("config: Could not find value in '%s'.\n", str);
        return;
    }

    cs++;

    register char *cr = str;

    while(*cs != '\n' && *cs != '\0' && *cs != '\r')
    {
        *cr = *cs;
        cs++,cr++;
    }

    *cr = '\0';
}



bool cfg_Open(const char *filepath)
{

    if((cfgfile = fopen(filepath, "r")) == NULL)
    {
        message_CriticalErrorEx("config: Could not open file '%s'.\n", filepath);
        return false;
    }

    return true;
} 

void cfg_Close()
{

    if(cfgfile != NULL)
        fclose(cfgfile);
    else
        message_Warning("config: Trying to close file, but none opened.\n");

}

bool cfg_SeekSection(char *sectionName)
{

    char buffer[_STR_BUFLEN];
    char pattern[_STR_BUFLEN];
    bool found = false;

    sprintf(pattern, "[%s]", sectionName);

    rewind(cfgfile);

    do fgets(buffer, _STR_BUFLEN, cfgfile);
    while(!feof(cfgfile) && !(found = checkPattern(pattern, buffer)));

    if(found)
    {
        section_offset = ftell(cfgfile);
        return true;
    }

    message_WarningEx("config: Could not find section '%s'.\n", pattern);

    return false;

}

bool cfg_NextSection(char *sectionName)
{
    char buffer[_STR_BUFLEN];
    char pattern[_STR_BUFLEN];
    int found = false;

    sprintf(pattern, "[");

    do fgets(buffer, _STR_BUFLEN, cfgfile);
    while(!feof(cfgfile) && !(found = checkPattern(pattern, buffer)));

    if(found)
    {
        char *start = buffer + 1;
        char *end = strrchr(start, ']') - 1;
        memcpy(sectionName, start, end - start + 1);
        sectionName[end - start + 1] = '\0';
        section_offset = ftell(cfgfile);
        return true;
    }

    return false;
}

bool cfg_GetIntValue(char *propertyName, int *value)
{

    char buffer[_STR_BUFLEN];

    if(cfg_GetStringValue(propertyName, buffer))
    {
        int val;
        if(sscanf(buffer, "%d", &val) < 1)
        {
            message_WarningEx("config: The value '%s=%s' is not an integer.\n", propertyName, buffer);
            return false;
        }

        *value = val;
        return true;
    }

    message_WarningEx("config: Could not find value '%s' within current section.\n", propertyName);

    return false;
}

bool cfg_GetDoubleValue(char *propertyName, double *value)
{

    char buffer[_STR_BUFLEN];

    if(cfg_GetStringValue(propertyName, buffer))
    {

        double val;

        if(sscanf(buffer, "%lf", &val) < 1)
        {
            message_WarningEx("config: The value '%s=%s' is not a double.\n", propertyName, buffer);
            return false;
        }

        *value = val;
        return true;
    }

    message_WarningEx("config: Could not find value '%s' within current section.\n", propertyName);
    return false;

}

bool cfg_FindPattern(char *pattern, char *buffer)
{
    bool found = false;

    fseek(cfgfile, section_offset, SEEK_SET);

    do fgets(buffer, _STR_BUFLEN, cfgfile);
    while(!feof(cfgfile) && !checkPattern("[", buffer) && !(found = checkPattern(pattern, buffer)));

    return found;
}

bool cfg_GetStringValue(char *propertyName, char *str)
{

    char buffer[_STR_BUFLEN];
    char pattern[_STR_BUFLEN];

    sprintf(pattern, "%s=", propertyName);

    if(cfg_FindPattern(pattern, buffer))
    {
        separateValue(buffer);
        trimString(buffer);
        strcpy(str, buffer);
        return true;
    }

    message_WarningEx("config: Could not find value '%s' within current section.\n", propertyName);
    return false;
}

bool cfg_GetTag(char *tagName)
{
    char buffer[_STR_BUFLEN];
    char pattern[_STR_BUFLEN];

    sprintf(pattern, "+%s", tagName);

    return cfg_FindPattern(pattern, buffer);
}

bool cfg_GetBool(char *propertyName)
{
    char buffer[_STR_BUFLEN];
    char pattern[_STR_BUFLEN];

    sprintf(pattern, "%s=", propertyName);

    if(cfg_FindPattern(pattern, buffer))
    {
        separateValue(buffer);
        trimString(buffer);

        if(strcmp(buffer, "1") == 0 || strcmp(buffer, "yes") == 0 ||
           strcmp(buffer, "y") == 0 || strcmp(buffer, "true") == 0 ||
           strcmp(buffer, "t") == 0)
            return true;
    }

    return false;
}

