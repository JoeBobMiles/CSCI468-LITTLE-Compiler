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

u64 copyString(char *dest, cchar *src) {
    assert(dest);
    assert(src);

    char *cur = dest;
    while ((*cur++ = *src++));

    return cur - dest - 1;
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
    StringTable **page = &globalStringTable;

    for (;;) {
        if (!*page) {
            *page = (StringTable *)malloc(sizeof **page);
            initStringTable(*page);
        }

        if (size < (**page).size - (**page).used) break;

        page = &(**page).nextPage;
    }

    char *strLoc = (**page).data + (**page).used;
    copyString(strLoc, str);
    (**page).used += size;

    assert((**page).size >= (**page).used);

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
