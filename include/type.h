#ifndef BASIL_TYPE_H
#define BASIL_TYPE_H

#include "defs.h"
#include "vec.h"
#include "hash.h"
#include "utf8.h"

namespace basil {
    enum class Kind {
        TYPE,
        NUMBER,
        FUNCTION,
        MACRO,
        ARRAY,
        SUM,
        INTERSECT,
        NAMED,
        RUNTIME
    };

    class Type {
        u32 _size;
    public:
        Type(u32 size);
        virtual ~Type();

        u32 size() const;
        virtual Kind kind() const;
        virtual ustring mangle() const;
        virtual bool implicitly(const Type* other) const;
        virtual bool explicitly(const Type* other) const;
        virtual void format(stream& io) const;
    };

    class NumberType : public Type {
        bool _float;
    public:
        NumberType(u32 size, bool floating);

        bool floating() const;
        Kind kind() const override;
        ustring mangle() const override;
        bool implicitly(const Type* other) const override;
        bool explicitly(const Type* other) const override;
        void format(stream& io) const override;
    };

    class FunctionType : public Type {
        vector<const Type*> _args;
        const Type* _ret;
    public:
        FunctionType(const vector<const Type*> args, const Type* ret);

        const vector<const Type*>& args() const;
        const Type* ret() const;
        Kind kind() const override;
        ustring mangle() const override;
        bool implicitly(const Type* other) const override;
        bool explicitly(const Type* other) const override;
        void format(stream& io) const override;
    };

    class MacroType : public Type {
        vector<const Type*> _args;
        const Type* _ret;
    public:
        MacroType(const vector<const Type*> args, const Type* ret);

        const vector<const Type*>& args() const;
        const Type* ret() const;
        Kind kind() const override;
        ustring mangle() const override;
        bool implicitly(const Type* other) const override;
        bool explicitly(const Type* other) const override;
        void format(stream& io) const override;
    };

    class ArrayType : public Type {
        const Type* _element;
        i64 _count;
    public:
        ArrayType(const Type* element);
        ArrayType(const Type* element, i64 count);

        const Type* element() const;
        i64 count() const;
        Kind kind() const override;
        ustring mangle() const override;
        bool implicitly(const Type* other) const override;
        bool explicitly(const Type* other) const override;
        void format(stream& io) const override;
    };

    class SumType : public Type {
        set<const Type*> _members;
    public:
        SumType(const set<const Type*>& members);

        const set<const Type*>& members() const;
        Kind kind() const override;
        ustring mangle() const override;
        bool implicitly(const Type* other) const override;
        bool explicitly(const Type* other) const override;
        void format(stream& io) const override;
    };

    class IntersectType : public Type {
        set<const Type*> _members;
    public:
        IntersectType(const set<const Type*>& members);

        const set<const Type*>& members() const;
        Kind kind() const override;
        ustring mangle() const override;
        bool implicitly(const Type* other) const override;
        bool explicitly(const Type* other) const override;
        void format(stream& io) const override;
    };

    class NamedType : public Type {
        const Type* _child;
        const ustring _name;
    public:
        NamedType(const Type* child, const ustring& name);
    
        const Type* child() const;
        const ustring& name() const;
        Kind kind() const override;
        ustring mangle() const override;
        bool implicitly(const Type* other) const override;
        bool explicitly(const Type* other) const override;
        void format(stream& io) const override;
    };

    class RuntimeType : public Type {
        const Type* _child;
    public:
        RuntimeType(const Type* child);

        const Type* child() const;
        Kind kind() const override;
        ustring mangle() const override;
        bool implicitly(const Type* other) const override;
        bool explicitly(const Type* other) const override;
        void format(stream& io) const override;
    };

    const Type* join(const Type* a, const Type* b);

    extern map<ustring, const Type*> TYPEMAP;

    template<typename T, typename ...Args>
    const Type* find(Args... args) {
        T* t = new T(args...);
        auto it = TYPEMAP.find(t->mangle());
        if (it != TYPEMAP.end()) {
            delete t;
            return it->second;
        }
        else {
            TYPEMAP[t->mangle()] = t;
            return t;
        }
    }

    extern const Type
        *INT, *FLOAT,
        *STRING, *CHAR,
        *SYMBOL, *ANY,
        *VOID, *TYPE, *BOOL,
        *UNDEFINED;
}

void write(stream& io, const basil::Type* t);

#endif