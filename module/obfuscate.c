#include "obfuscate.h"


char * obfuscate(char * plane, char * obfuscated, int size) {
    return strncpy(obfuscated, plane, size);
}

char * deobfuscate(char * obfuscated, char * plane, int size) {
    return strncpy(plane, obfuscated, size);

}