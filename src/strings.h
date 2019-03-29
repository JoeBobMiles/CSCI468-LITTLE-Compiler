#ifndef strings_h
#define strings_h

#include "main.h"

u64 stringLength(cchar *str);
u64 stringSize(cchar *str);
void copyString(char *dest, cchar *src);
bool stringsAreEqual(cchar *a, cchar *b);
void zeroMemory(char *mem, u64 size);



#define PAGESIZE (4*1024) /* 4kB of strings should be enough for anyone */
struct StringTable {
    StringTable *nextPage; /* but we'll have multiple pages just in case */
    u32 used;
    u32 size;
    char *data;
};
extern StringTable *globalStringTable;

char *saveString(cchar *str);

void initStringTable(StringTable *table);
void deinitStringTable(StringTable *table);

#endif
