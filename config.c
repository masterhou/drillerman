#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"

static FILE* cfgfile = NULL;
static long section_offset = 0;

static void separate_value(char* str);
static int check_pattern(char* pattern, char* str);

int cfgOpen(const char *filepath)
{

    if((cfgfile = fopen(filepath, "r")) == NULL)
        messageCriticalErrorEx("config: Could not open file '%s'.\n", filepath);

    return 1;
} 

void cfgClose()
{

    if(cfgfile != NULL)
        fclose(cfgfile);
    else
        messageWarning("config: Trying to close file, but none opened.\n");

}

int cfgSeekSection(char *section_name)
{

    char buffer[256];
    char pattern[256];
    int found = 0;

    sprintf(pattern, "[%s]", section_name);

    rewind(cfgfile);

    do fgets(buffer, 256, cfgfile);
    while(!feof(cfgfile) && !(found = check_pattern(pattern, buffer)));

    if(found)
    {
        section_offset = ftell(cfgfile);
        return 1;
    }

    messageWarningEx("config: Could not find section '%s'.\n", pattern);

    return 0;

}

int cfgNextSection(char *section_name)
{

    char buffer[256];
    char pattern[256];
    int found = 0;

    sprintf(pattern, "[");

    do fgets(buffer, 256, cfgfile);
    while(!feof(cfgfile) && !(found = check_pattern(pattern, buffer)));

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

int cfgGetIntValue(char *value_name, int *value)
{

    char buffer[256];

    if(cfgGetStringValue(value_name, buffer))
    {
        int val;
        if(sscanf(buffer, "%d", &val) < 1)
        {
            messageWarningEx("config: The value '%s=%s' is not an integer.\n", value_name, buffer);
            return 0;
        }

        *value = val;
        return 1;
    }

    messageWarningEx("config: Could not find value '%s' within current section.\n", value_name);

    return 0;

}

int cfgGetDoubleValue(char *value_name, double *value)
{

    char buffer[256];

    if(cfgGetStringValue(value_name, buffer))
    {

        double val;

        if(sscanf(buffer, "%lf", &val) < 1)
        {
            messageWarningEx("config: The value '%s=%s' is not a double.\n", value_name, buffer);
            return 0;
        }

        *value = val;
        return 1;
    }

    messageWarningEx("config: Could not find value '%s' within current section.\n", value_name);
    return 0;

}

int cfgGetStringValue(char *value_name, char *str)
{

    char buffer[256];
    char pattern[256];
    int found = 0;

    sprintf(pattern, "%s=", value_name);

    fseek(cfgfile, section_offset, SEEK_SET);


    do fgets(buffer, 255, cfgfile);
    while(!feof(cfgfile) && !check_pattern("[", buffer) && !(found = check_pattern(pattern, buffer)));

    if(found)
    {
        separate_value(buffer);
        strcpy(str, buffer);

        return 1;
    }

    messageWarningEx("config: Could not find value '%s' within current section.\n", value_name);
    return 0;
}

int cfgGetTag(char *tag_name)
{
    char buffer[256];
    char pattern[256];
    int found = 0;

    sprintf(pattern, "%s=", tag_name);

    fseek(cfgfile, section_offset, SEEK_SET);


    do fgets(buffer, 255, cfgfile);
    while(!feof(cfgfile) && !check_pattern("[", buffer) && !(found = check_pattern(pattern, buffer)));

    return found;
}

static int check_pattern(char *pattern, char *str)
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

static void separate_value(char *str)
{

    int i, j;

    i = 0;

    while(str[i] != '=') ++i;

    if(i == strlen(str))
    {
        messageWarningEx("config: Could not find value in '%s'.\n", str);
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

