#ifndef __OBFUSCATE_H
#define __OBFUSCATE_H
#include <linux/string.h>

#ifndef OBFUSCATION_KEY
#warning Missing obfuscation key. Please set using -DOBFUSCANTION_KEY=OBFUSCATION_KEY
#define OBFUSCATION_KEY 0xd1, 0x7f, 0x07, 0xe8, 0xbf, 0x65, 0xc4, 0xe1, 0x1f, 0x49, 0x86, 0xcf, 0x45, 0xb7, 0x3a, 0x36, 0xa8, 0x6e, 0xd1, 0xa5, 0xa0, 0xe1, 0xab, 0x76, 0x23, 0xb7, 0x26, 0x59, 0x92, 0x19, 0x66, 0x00
#endif

int obfuscate(char *plane, char *obfuscated);
int deobfuscate(char *obfuscated, char *plane);

#endif