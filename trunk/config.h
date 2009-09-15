#ifndef CONFIG_H
#define CONFIG_H


int cfg_Open(const char *filepath);
void cfg_Close();

int cfg_SeekSection(char *section_name);
int cfg_NextSection(char *section_name);

int cfg_GetIntValue(char *value_name, int *value);
int cfg_GetDoubleValue(char *value_name, double *value);
int cfg_GetStringValue(char *value_name, char *str);
int cfg_GetTag(char *tag_name);



#endif

