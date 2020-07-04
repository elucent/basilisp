#include "type.h"
#include "hash.h"
#include "io.h"

namespace basil {

    // Type

    Type::Type(u32 size):
        _size(size) {
        //
    }

    Type::~Type() {
        //
    }

    u32 Type::size() const {
        return _size;
    }

    Kind Type::kind() const {
        return Kind::TYPE;
    }

    ustring Type::mangle() const {
        buffer b; ustring s;
        write(b, "@", _size * 8);
        read(b, s);
        return s;
    }

    bool Type::implicitly(const Type* other) const {
        return other == this || other == ANY || 
            (other->kind() == Kind::RUNTIME && 
                ((RuntimeType*)other)->child() == this) ||
            (other->kind() == Kind::SUM && 
                ((SumType*)other)->members().find(this) 
                 != ((SumType*)other)->members().end());
    }

    bool Type::explicitly(const Type* other) const {
        return implicitly(other);
    }

    void Type::format(stream& io) const {
        write(io, "@", _size * 8);
    }

    // NumberType
    
    NumberType::NumberType(u32 size, bool floating):
        Type(size), _float(floating) {
        //
    }

    bool NumberType::floating() const {
        return _float;
    }

    Kind NumberType::kind() const {
        return Kind::NUMBER;
    }

    ustring NumberType::mangle() const {
        buffer b; ustring s;
        write(b, floating() ? "F" : "I", size() * 8);
        read(b, s);
        return s;
    }

    bool NumberType::implicitly(const Type* other) const {
        if (Type::implicitly(other)) return true;
        if (other->kind() != this->kind()) return false;
        return ((NumberType*)other)->floating() == floating();
    }

    bool NumberType::explicitly(const Type* other) const {
        return other->kind() == this->kind();
    }

    void NumberType::format(stream& io) const {
        write(io, floating() ? "f" : "i", size() * 8);
    }

    // FunctionType
    
    FunctionType::FunctionType(const vector<const Type*> args, const Type* ret):
        Type(8), _args(args), _ret(ret) {
        //
    }

    const vector<const Type*>& FunctionType::args() const {
        return _args;
    }

    const Type* FunctionType::ret() const {
        return _ret;
    }

    Kind FunctionType::kind() const {
        return Kind::FUNCTION;
    }

    ustring FunctionType::mangle() const {
        buffer b; ustring s;
        write(b, "L", _ret->mangle(), "(");
        for (const Type* t : _args) {
            write(b, t->mangle());
            if (t != _args.back()) write(b, ",");
        }
        write(b, ")");
        read(b, s);
        return s;
    }

    bool FunctionType::implicitly(const Type* other) const {
        return Type::implicitly(other);
    }

    bool FunctionType::explicitly(const Type* other) const {
        return implicitly(other);
    }

    void FunctionType::format(stream& io) const {
        write(io, "(function ");
        for (const Type* const & t : _args) {
            write(io, t);
            if (&t != &_args.back()) write(io, " ");
        }
        write(io, " -> ", _ret, ")");
    }

    // MacroType
    
    MacroType::MacroType(const vector<const Type*> args, const Type* ret):
        Type(0), _args(args), _ret(ret) {
        //
    }

    const vector<const Type*>& MacroType::args() const {
        return _args;
    }

    const Type* MacroType::ret() const {
        return _ret;
    }

    Kind MacroType::kind() const {
        return Kind::MACRO;
    }

    ustring MacroType::mangle() const {
        buffer b; ustring s;
        write(b, "M", _ret->mangle(), "(");
        for (const Type* t : _args) {
            write(b, t->mangle());
            if (t != _args.back()) write(b, ",");
        }
        write(b, ")");
        read(b, s);
        return s;
    }

    bool MacroType::implicitly(const Type* other) const {
        return Type::implicitly(other);
    }

    bool MacroType::explicitly(const Type* other) const {
        return implicitly(other);
    }

    void MacroType::format(stream& io) const {
        write(io, "(macro ");
        for (const Type* const & t : _args) {
            write(io, t);
            if (&t != &_args.back()) write(io, " ");
        }
        write(io, " -> ", _ret, ")");
    }

    // ArrayType

    ArrayType::ArrayType(const Type* element):
        Type(8), _element(element), _count(-1) {
        //
    }

    ArrayType::ArrayType(const Type* element, i64 count):
        Type(element->size() * count), _element(element), _count(count) {
        //
    }

    const Type* ArrayType::element() const {
        return _element;
    }

    i64 ArrayType::count() const {
        return _count;
    }

    Kind ArrayType::kind() const {
        return Kind::ARRAY;
    }

    ustring ArrayType::mangle() const {
        buffer b; ustring s;
        write(b, "A", _element->mangle(), "[");
        if (_count > -1) write(b, _count);
        write(b, "]");
        read(b, s);
        return s;
    }

    bool ArrayType::implicitly(const Type* other) const {
        return Type::implicitly(other) || 
            (other->kind() == Kind::ARRAY && ((ArrayType*)other)->count() == -1);
    }

    bool ArrayType::explicitly(const Type* other) const {
        return implicitly(other);
    }

    void ArrayType::format(stream& io) const {
        write(io, "(", _element, " [");
        if (_count > -1) write(io, _count);
        write(io, "])");
    }

    template<typename Container>
    u32 totalsize(const Container& c) {
        u32 size = 0;
        for (const Type* t : c) size += t->size();
        return size;
    }

    template<typename Container>
    u32 maxsize(const Container& c) {
        u32 size = 0;
        for (const Type* t : c) size = size < t->size() ? t->size() : size;
        return size;
    }
    
    SumType::SumType(const set<const Type*>& members):
        Type(totalsize(members)), _members(members) {
        //
    }

    const set<const Type*>& SumType::members() const {
        return _members;
    }

    Kind SumType::kind() const {
        return Kind::SUM;
    }

    ustring SumType::mangle() const {
        buffer b; ustring s;
        write(b, "|(");
        for (const Type* t : _members) {
            write(b, t->mangle(), ","); 
        }
        write(b, ")");
        read(b, s);
        return s;
    }

    bool SumType::implicitly(const Type* other) const {
        return Type::implicitly(other);
    }

    bool SumType::explicitly(const Type* other) const {
        return implicitly(other) || _members.find(other) != _members.end();
    }

    void SumType::format(stream& io) const {
        write(io, "(union");
        for (const Type* t : _members) {
            write(io, " ", t); 
        }
        write(io, ")");
    }
    
    IntersectType::IntersectType(const set<const Type*>& members):
        Type(maxsize(members)), _members(members) {
        //
    }

    const set<const Type*>& IntersectType::members() const {
        return _members;
    }

    Kind IntersectType::kind() const {
        return Kind::INTERSECT;
    }

    ustring IntersectType::mangle() const {
        buffer b; ustring s;
        write(b, "&(");
        for (const Type* t : _members) {
            write(b, t->mangle(), ","); 
        }
        write(b, ")");
        read(b, s);
        return s;
    }

    bool IntersectType::implicitly(const Type* other) const {
        return Type::implicitly(other) || _members.find(other) != _members.end();
    }

    bool IntersectType::explicitly(const Type* other) const {
        return implicitly(other);
    }

    void IntersectType::format(stream& io) const {
        write(io, "(intersect");
        for (const Type* t : _members) {
            write(io, " ", t); 
        }
        write(io, ")");
    }
    
    NamedType::NamedType(const Type* child, const ustring& name):
        Type(child->size()), _child(child), _name(name) {
        //
    }

    const Type* NamedType::child() const {
        return _child;
    }

    const ustring& NamedType::name() const {
        return _name;
    }

    Kind NamedType::kind() const {
        return Kind::NAMED;
    }

    ustring NamedType::mangle() const {
        buffer b; ustring s;
        write(b, "N", _name, _child->mangle());
        read(b, s);
        return s;
    }

    bool NamedType::implicitly(const Type* other) const {
        return Type::implicitly(other);
    }

    bool NamedType::explicitly(const Type* other) const {
        return implicitly(other) || _child->explicitly(other);
    }

    void NamedType::format(stream& io) const {
        write(io, _name);
    }

    RuntimeType::RuntimeType(const Type* child):
        Type(child->size()), _child(child) {
        //
    }

    const Type* RuntimeType::child() const {
        return _child;
    }

    Kind RuntimeType::kind() const {
        return Kind::RUNTIME;
    }

    ustring RuntimeType::mangle() const {
        buffer b; ustring s;
        write(b, "?", _child->mangle());
        read(b, s);
        return s;
    }

    bool RuntimeType::implicitly(const Type* other) const {
        return _child->implicitly(other);
    }

    bool RuntimeType::explicitly(const Type* other) const {
        return _child->explicitly(other);
    }

    void RuntimeType::format(stream& io) const {
        write(io, "(runtime ", _child, ")");
    }

    const Type* join(const Type* a, const Type* b) {
        if (a == UNDEFINED || b == UNDEFINED) return UNDEFINED;
        if (a == b) return a;
        else if (b->kind() == Kind::RUNTIME && a->implicitly(b))
            return b;
        else if (a->kind() == Kind::RUNTIME && b->implicitly(a))
            return a;
        else if (a->implicitly(b)) return b;
        else if (b->implicitly(a)) return a;
        else if (a->explicitly(b)) return b;
        else if (b->explicitly(a)) return a;
        else return nullptr;
    }

    map<ustring, const Type*> TYPEMAP;

    const Type
        *INT = find<NumberType>(8, false), 
        *FLOAT = find<NumberType>(8, true),
        *STRING = find<NamedType>(find<Type>(8), "string"), 
        *CHAR = find<NamedType>(find<Type>(4), "char"),
        *SYMBOL = find<NamedType>(find<Type>(4), "symbol"),
        *ANY = find<NamedType>(find<Type>(0), "any"),
        *VOID = find<NamedType>(find<Type>(0), "void"),
        *TYPE = find<NamedType>(find<Type>(4), "type"),
        *BOOL = find<NamedType>(find<Type>(1), "bool"),
        *UNDEFINED = find<NamedType>(find<Type>(0), "undefined");
}

void write(stream& io, const basil::Type* t) {
    t->format(io);
}
