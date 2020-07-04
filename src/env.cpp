#include "env.h"

namespace basil {
    Env::Env():
        _parent(nullptr) {
        //
    }

    void Env::setParent(Env* parent) {
        _parent = parent;
    }

    Env* Env::parent() const {
        return _parent;
    }

    Entry* Env::entry(u32 i) {
        return &_entryOrder[i]->second;
    }

    const Entry* Env::entry(u32 i) const {
        return &_entryOrder[i]->second;
    }

    Entry* Env::lookup(const ustring& name) {
        auto it = _entries.find(name);
        if (it != _entries.end()) return &it->second;
        else if (_parent) return _parent->lookup(name);
        else return nullptr;
    }

    const Entry* Env::lookup(const ustring& name) const {
        auto it = _entries.find(name);
        if (it != _entries.end()) return &it->second;
        else if (_parent) return _parent->lookup(name);
        else return nullptr;
    }

    void Env::enter(const ustring& name, const Meta& meta) {
        auto it = _entries.find(name);
        if (it != _entries.end()) it->second.meta = meta;
        else {
            _entries.put(name, { meta });
            auto nit = _entries.find(name);
            _entryOrder.push(&*nit);
        }
    }

    Env* Env::fork() const {
        Env* result = new Env();
        result->setParent(_parent);

        for (auto e : _entryOrder) {
            result->_entries.put(e->first, e->second);
            result->_entryOrder.push(&*result->_entries.find(e->first));
        }
        return result;
    }

    map<ustring, Entry>& Env::entries() {
        return _entries;
    }

    const map<ustring, Entry>& Env::entries() const {
        return _entries;
    }
}