#ifndef BASIL_META_H
#define BASIL_META_H

#include "defs.h"
#include "vec.h"
#include "hash.h"
#include "utf8.h"
#include "io.h"

namespace basil {
    extern i64 findSymbol(const ustring& name);
    extern const ustring& findSymbol(i64 name);

    class Meta {
        const Type* _type;
        union {
            i64 i;
            double d;
            char c[4];
            const Type* t;
            bool b;
            MetaString* s;
            MetaArray* a;
            MetaUnion* un;
            MetaIntersect* in;
            MetaFunction* f;
            Node* rt;
        } value;

        void free();
        void copy(const Meta& other);
        void assign(const Meta& other);
    public:
        Meta();
        Meta(const Type* type);
        Meta(const Type* type, i64 i);
        Meta(const Type* type, double d);
        Meta(const Type* type, uchar c);
        Meta(const Type* type, const Type* t);
        Meta(const Type* type, bool b);
        Meta(const Type* type, const ustring& s);
        Meta(const Type* type, MetaArray* a);
        Meta(const Type* type, MetaUnion* un);
        Meta(const Type* type, MetaIntersect* in);
        Meta(const Type* type, MetaFunction* f);
        Meta(const Type* type, Node* rt);
        ~Meta();
        Meta(const Meta& other);
        Meta& operator=(const Meta& other);
        const Type* type() const;
        bool isVoid() const;
        bool isInt() const;
        i64 asInt() const;
        i64& asInt();
        bool isFloat() const;
        double asFloat() const;
        double& asFloat();
        bool isChar() const;
        uchar asChar() const;
        uchar& asChar();
        bool isType() const;
        const Type* asType() const;
        const Type*& asType();
        bool isBool() const;
        bool asBool() const;
        bool& asBool();
        bool isSymbol() const;
        i64 asSymbol() const;
        i64& asSymbol();
        bool isString() const;
        const ustring& asString() const;
        ustring& asString();
        bool isArray() const;
        const MetaArray& asArray() const;
        MetaArray& asArray();
        bool isUnion() const;
        const MetaUnion& asUnion() const;
        MetaUnion& asUnion();
        bool isIntersect() const;
        const MetaIntersect& asIntersect() const;
        MetaIntersect& asIntersect();
        bool isFunction() const;
        const MetaFunction& asFunction() const;
        MetaFunction& asFunction();
        bool isRuntime() const;
        Node* asRuntime() const;
        Node*& asRuntime();
        operator bool() const;
        Meta clone() const;
        void format(stream& io) const;
        bool operator==(const Meta& fr) const;
        bool operator!=(const Meta& fr) const;
        ustring toString() const;
        u64 hash() const;
    };

    class MetaRC {
        u32 rc;
    public:
        MetaRC();
        virtual ~MetaRC();
        void inc();
        void dec();
        virtual Meta clone(const Meta& src) const = 0;
    };

    class MetaString : public MetaRC {
        ustring s;
    public:
        MetaString(const ustring& str);
        const ustring& str() const;
        ustring& str();
        Meta clone(const Meta& src) const override;
    };

    class MetaArray : public MetaRC {
        vector<Meta> vals;
    public:
        MetaArray(const vector<Meta>& values);
        const Meta& operator[](u32 i) const;
        Meta& operator[](u32 i);
        const Meta* begin() const;
        const Meta* end() const;
        Meta* begin();
        Meta* end();
        u32 size() const;
        Meta clone(const Meta& src) const override;
    };

    class MetaUnion : public MetaRC {
        Meta real;
    public:
        MetaUnion(const Meta& val);
        bool is(const Type* t) const;
        const Meta& value() const;
        Meta& value();
        Meta clone(const Meta& src) const override;
    };

    class MetaIntersect : public MetaRC {
        vector<Meta> vals;
    public:
        MetaIntersect(const vector<Meta>& values);
        const Meta& as(const Type* t) const;
        Meta& as(const Type* t);
        const Meta* begin() const;
        const Meta* end() const;
        Meta* begin();
        Meta* end();
        u32 size() const;
        Meta clone(const Meta& src) const override;
    };

    class MetaFunction : public MetaRC {
        Node* fn;
        Builtin _builtin;
        vector<u32> _args;
        Env* _local;
    public:
        MetaFunction(Node* function, const vector<u32>& args, Env* local);
        MetaFunction(Builtin builtin);
        ~MetaFunction();
        Node* function() const;
        Builtin builtin() const;
        Entry* arg(u32 i) const;
        Env* local() const;
        Meta clone(const Meta& src) const override;
    };

    i64 trunc(i64 n, const Type* dest);
    u64 trunc(u64 n, const Type* dest);
    double toFloat(const Meta& m);
    i64 toInt(const Meta& m);
    u64 toUint(const Meta& m);

    Meta add(const Meta& lhs, const Meta& rhs);
    Meta sub(const Meta& lhs, const Meta& rhs);
    Meta mul(const Meta& lhs, const Meta& rhs);
    Meta div(const Meta& lhs, const Meta& rhs);
    Meta mod(const Meta& lhs, const Meta& rhs);
    Meta andf(const Meta& lhs, const Meta& rhs);
    Meta orf(const Meta& lhs, const Meta& rhs);
    Meta xorf(const Meta& lhs, const Meta& rhs);
    Meta notf(const Meta& operand);
    Meta equal(const Meta& lhs, const Meta& rhs);
    Meta inequal(const Meta& lhs, const Meta& rhs);
    Meta less(const Meta& lhs, const Meta& rhs);
    Meta lessequal(const Meta& lhs, const Meta& rhs);
    Meta greater(const Meta& lhs, const Meta& rhs);
    Meta greaterequal(const Meta& lhs, const Meta& rhs);
    Meta unionf(const Meta& lhs, const Meta& rhs);
    Meta intersect(const Meta& lhs, const Meta& rhs);
    void assign(Meta& lhs, const Meta& rhs);
    Meta cast(const Meta& lhs, const Type* dst);
}

template<>
u64 hash(const basil::Meta& m);

void write(stream& io, const basil::Meta& m);

#endif