#ifndef __OBFUSCATE_H
#define __OBFUSCATE_H
#include <linux/string.h>
#define OBFUSCATION_KEY "TwojaStaraSieJaraTwojaStaraSieJara12332"

char * obfuscate(char * plane, char * obfuscated, int size);
char * deobfuscate(char * obfuscated, char * plane, int size);

#endif