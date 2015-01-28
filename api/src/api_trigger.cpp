#include "stdafx.h"
#include "../api.h"
#include "trigger.h"

Trigger* tcast(trigger t) { return (Trigger*)t; }
trigger trigger_create(const utf8* begin, const utf8* end, int max_len)
{
    Trigger *t = new Trigger();
    if (!t->init(begin, end, max_len))
        { delete t;  t = NULL; }
    return t;
}

void trigger_delete(trigger t)
{
    delete tcast(t);
}

bool trigger_addstream(trigger t, const utf8* data)
{
    if (!t) return false;
    return tcast(t)->add(data);
}

const utf8* trigger_find(trigger t, const utf8* begin, const utf8* end)
{
    if (!t) return NULL;
    return tcast(t)->find(begin, end);
}
