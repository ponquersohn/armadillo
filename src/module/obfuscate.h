#ifndef __OBFUSCATE_H
#define __OBFUSCATE_H
#include <linux/string.h>

#ifndef OBFUSCATION_KEY
#warning Missing obfuscation key. Please set using -DOBFUSCANTION_KEY=OBFUSCATION_KEY
#define OBFUSCATION_KEY "1234567890123456678909001234567890123456"
#endif


char * obfuscate(char * plane, char * obfuscated, int size);
char * deobfuscate(char * obfuscated, char * plane, int size);

#endif