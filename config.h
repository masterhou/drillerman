#ifndef CONFIG_H
#define CONFIG_H


int cfgOpen(const char *filepath);
void cfgClose();

int cfgSeekSection(char *section_name);
int cfgNextSection(char *section_name);

int cfgGetIntValue(char *value_name, int *value);
int cfgGetDoubleValue(char *value_name, double *value);
int cfgGetStringValue(char *value_name, char *str);
int cfgGetTag(char *tag_name);



#endif

