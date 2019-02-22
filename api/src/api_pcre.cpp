#include "stdafx.h"
#include "../api.h"
#include <vector>

#include "pcreHelper.h"
#include "memoryBuffer.h"

hpcre pcre_create(const wchar_t* regexp)
{
    if (!regexp || !wcslen(regexp))
        return NULL;
    Pcre16 *h = new Pcre16();
    if (!h->setRegExp(regexp, true))
        {  delete h; return NULL; }
   return h;
}

void pcre_delete(hpcre handle)
{
    if (!handle) return;
    Pcre16* h = (Pcre16*)handle;
    delete h;  
}

bool pcre_find(hpcre handle, const wchar_t* string)
{
    if (!handle || !string) return false;
    Pcre16* h = (Pcre16*)handle;
    h->find(string);
    return (h->getSize() == 0) ? false : true;
}

bool pcre_findall(hpcre handle, const wchar_t* string)
{
    if (!handle || !string) return false;
    Pcre16* h = (Pcre16*)handle;
    h->findAllMatches(string);
    return (h->getSize() == 0) ? false : true;
}

int pcre_size(hpcre handle)
{
    if (!handle) return 0;
    Pcre16* h = (Pcre16*)handle;
    return h->getSize();
}

int pcre_first(hpcre handle, int index)
{
    if (!handle) return -1;
    Pcre16* h = (Pcre16*)handle;
    return h->getFirst(index);
}

int pcre_last(hpcre handle, int index)
{
    if (!handle) return -1;
    Pcre16* h = (Pcre16*)handle;
    return h->getLast(index);
}

hpcre_string pcre_string(hpcre handle, int index)
{
    if (!handle) return NULL;
    Pcre16* h = (Pcre16*)handle;
    std::wstring text;
    if (!h->getString(index, &text))
        return NULL;
    int len = (text.length()+1)*sizeof(wchar_t);
    strbuf b = strbuf_new(len);
    wcscpy((wchar_t*)strbuf_ptr(b), text.c_str());
    return b;
}

hpcre_string pcre_regexp(hpcre handle)
{
    if (!handle) return NULL;
    Pcre16* h = (Pcre16*)handle;
    std::wstring regexp;
    h->getRegexp(&regexp);
    int len = (regexp.length()+1)*sizeof(wchar_t);
    strbuf b = strbuf_new(len);
    wcscpy((wchar_t*)strbuf_ptr(b), regexp.c_str());
    return b;
}
