#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "defs.h"

static FILE* cfgfile = NULL;
static long section_offset = 0;

static void separateValue(char* str);
static int checkPattern(char* pattern, char* str);

static int checkPattern(char *pattern, char *str)
{

    if((strlen(pattern) < 1) || (strlen(str) < 1)) return 0;

    int i, j;

    i = 0;

    while((str[i] == ' ') && (str[i] != '\0')) ++i;

    j = 0;

    while((str[i] != '\0') && (pattern[j] != '\0') && (str[i] == pattern[j])) i++,j++;

    if(j == strlen(pattern)) return 1;

    return 0;

}

static void separateValue(char *str)
{

    int i, j;

    i = 0;

    while(str[i] != '=') ++i;

    if(i == strlen(str))
    {
        message_WarningEx("config: Could not find value in '%s'.\n", str);
        return;
    }

    j = 0;

    while(str[i] != '\n' && str[i] != 0 && str[i] != ' ' )
    {
        ++i;
        str[j] = str[i];
        ++j;
    }

    str[j - 1] = '\0';
}



int cfg_Open(const char *filepath)
{

    if((cfgfile = fopen(filepath, "r")) == NULL)
        message_CriticalErrorEx("config: Could not open file '%s'.\n", filepath);

    return 1;
} 

void cfg_Close()
{

    if(cfgfile != NULL)
        fclose(cfgfile);
    else
        message_Warning("config: Trying to close file, but none opened.\n");

}

int cfg_SeekSection(char *section_name)
{

    char buffer[_STR_BUFLEN];
    char pattern[_STR_BUFLEN];
    int found = 0;

    sprintf(pattern, "[%s]", section_name);

    rewind(cfgfile);

    do fgets(buffer, _STR_BUFLEN, cfgfile);
    while(!feof(cfgfile) && !(found = checkPattern(pattern, buffer)));

    if(found)
    {
        section_offset = ftell(cfgfile);
        return 1;
    }

    message_WarningEx("config: Could not find section '%s'.\n", pattern);

    return 0;

}

int cfg_NextSection(char *section_name)
{

    char buffer[_STR_BUFLEN];
    char pattern[_STR_BUFLEN];
    int found = 0;

    sprintf(pattern, "[");

    do fgets(buffer, _STR_BUFLEN, cfgfile);
    while(!feof(cfgfile) && !(found = checkPattern(pattern, buffer)));

    if(found)
    {
        char *start = buffer + 1;
        char *end = strrchr(start, ']') - 1;
        memcpy(section_name, start, end - start + 1);


        section_name[end - start + 1] = '\0';

        section_offset = ftell(cfgfile);
        return 1;
    }

    return 0;

}

int cfg_GetIntValue(char *valueName, int *value)
{

    char buffer[_STR_BUFLEN];

    if(cfg_GetStringValue(valueName, buffer))
    {
        int val;
        if(sscanf(buffer, "%d", &val) < 1)
        {
            message_WarningEx("config: The value '%s=%s' is not an integer.\n", valueName, buffer);
            return 0;
        }

        *value = val;
        return 1;
    }

    message_WarningEx("config: Could not find value '%s' within current section.\n", valueName);

    return 0;

}

int cfg_GetDoubleValue(char *value_name, double *value)
{

    char buffer[256];

    if(cfg_GetStringValue(value_name, buffer))
    {

        double val;

        if(sscanf(buffer, "%lf", &val) < 1)
        {
            message_WarningEx("config: The value '%s=%s' is not a double.\n", value_name, buffer);
            return 0;
        }

        *value = val;
        return 1;
    }

    message_WarningEx("config: Could not find value '%s' within current section.\n", value_name);
    return 0;

}

int cfg_GetStringValue(char *value_name, char *str)
{

    char buffer[_STR_BUFLEN];
    char pattern[_STR_BUFLEN];
    int found = 0;

    sprintf(pattern, "%s=", value_name);

    fseek(cfgfile, section_offset, SEEK_SET);


    do fgets(buffer, _STR_BUFLEN, cfgfile);
    while(!feof(cfgfile) && !checkPattern("[", buffer) && !(found = checkPattern(pattern, buffer)));

    if(found)
    {
        separateValue(buffer);
        strcpy(str, buffer);

        return 1;
    }

    message_WarningEx("config: Could not find value '%s' within current section.\n", value_name);
    return 0;
}

int cfg_GetTag(char *tag_name)
{
    char buffer[_STR_BUFLEN];
    char pattern[_STR_BUFLEN];
    int found = 0;

    sprintf(pattern, "%s=", tag_name);

    fseek(cfgfile, section_offset, SEEK_SET);


    do fgets(buffer, _STR_BUFLEN, cfgfile);
    while(!feof(cfgfile) && !checkPattern("[", buffer) && !(found = checkPattern(pattern, buffer)));

    return found;
}

