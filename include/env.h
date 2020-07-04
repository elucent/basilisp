#ifndef BASIL_ENV_H
#define BASIL_ENV_H

#include "defs.h"
#include "hash.h"
#include "meta.h"

namespace basil {
    struct Entry {
        Meta meta;
    };
    
    class Env {
        map<ustring, Entry> _entries;
        vector<pair<ustring, Entry>*> _entryOrder;
        Env* _parent;
    public:
        Env();

        void setParent(Env* parent);
        Env* parent() const;
        Entry* entry(u32 i);
        const Entry* entry(u32 i) const;
        Entry* lookup(const ustring& name);
        const Entry* lookup(const ustring& name) const;
        void enter(const ustring& name, const Meta& meta);
        Env* fork() const;
        map<ustring, Entry>& entries();
        const map<ustring, Entry>& entries() const;
    };
}

#endif