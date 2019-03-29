#include "main.h"
#include "strings.h"

#include <cstdlib>

u64 stringLength(cchar *str) {
    assert(str);

    cchar *cur = str;
    while (*cur) ++cur;
    return cur - str;
}

u64 stringSize(cchar *str) {
    assert(str);

    cchar *cur = str;
    while (*cur++);
    return cur - str;
}

void copyString(char *dest, cchar *src) {
    assert(dest);
    assert(src);

    do {
        *dest++ = *src;
    }
    while (*src++);
}

bool stringsAreEqual(cchar *a, cchar *b) {
    assert(a);
    assert(b);

    while (*a && *a == *b) {
        ++a; ++b;
    }
    return (*a == *b);
}

void zeroMemory(char *mem, u64 size) {
    assert(mem);

    char *end = mem + size;
    while (mem < end) {
        *mem++ = 0;
    }
}



StringTable *globalStringTable = 0; /* declare here */

char *saveString(cchar *str) {
    u64 size = stringSize(str);
    assert(size < PAGESIZE);

    /* TODO: save to other than global? */
    StringTable **pagePtr = &globalStringTable;

    for (;;) {
        if (!*pagePtr) {
            *pagePtr = (StringTable *)malloc(sizeof **pagePtr);
            initStringTable(*pagePtr);
        }

        if (size < (**pagePtr).size - (**pagePtr).used) break;

        pagePtr = &(**pagePtr).nextPage;
    }

    char *strLoc = (**pagePtr).data + (**pagePtr).used;
    copyString(strLoc, str);
    (**pagePtr).used += size;

    assert((**pagePtr).size >= (**pagePtr).used);

    return strLoc;
}

void initStringTable(StringTable *page) {
    assert(page);

    *page = (StringTable){
        .nextPage = 0,
        .used = 0,
        .size = PAGESIZE,
        .data = (char *)malloc(PAGESIZE)
    };
}

void deinitStringTable(StringTable *page) {
    while (page) {
        StringTable *nextPage = page->nextPage;

        free(page->data);
        free(page);

        page = nextPage;
    }
}
