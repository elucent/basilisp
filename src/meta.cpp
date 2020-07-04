#include "meta.h"
#include "type.h"
#include "io.h"
#include "errors.h"
#include "env.h"

namespace basil {
    map<i64, ustring> symbolnames;
    map<ustring, i64> symbolids;
    i64 next = 0;

    i64 findSymbol(const ustring& name) {
        auto it = symbolids.find(name);
        if (it == symbolids.end()) {
            i64 id = next ++;
            symbolnames[symbolids[name] = id] = name;
            return id;
        }
        return it->second;
    }

    static const ustring NO_SYMBOL;

    const ustring& findSymbol(i64 id) {
        auto it = symbolnames.find(id);
        if (it == symbolnames.end()) {
            return NO_SYMBOL;
        }
        return it->second;
    }

    // Meta

    void Meta::free() {
        if (isString()) value.s->dec();
        else if (isArray()) value.a->dec();
        else if (isUnion()) value.un->dec();
        else if (isIntersect()) value.in->dec();
        else if (isFunction()) value.f->dec();
    }

    void Meta::copy(const Meta& other) {
        _type = other._type;
        if (!_type || isVoid()) return;
        else if (isInt()) value.i = other.value.i;
        else if (isFloat()) value.d = other.value.d;
        else if (isType()) value.t = other.value.t;
        else if (isBool()) value.b = other.value.b;
        else if (isSymbol()) value.i = other.value.i;
        else if (isString()) value.s = other.value.s, value.s->inc();
        else if (isArray()) value.a = other.value.a, value.a->inc();
        else if (isUnion()) value.un = other.value.un, value.un->inc();
        else if (isIntersect()) value.in = other.value.in, value.in->inc();
        else if (isFunction()) value.f = other.value.f, value.f->inc();
        else if (isRuntime()) value.rt = other.value.rt;
    }

    void Meta::assign(const Meta& other) {
        if (!_type) return copy(other);
        auto prev = value;
        _type = other._type;
        value.s = nullptr;
        if (!_type || isVoid()) return;
        else if (isInt()) value.i = other.value.i;
        else if (isFloat()) value.d = other.value.d;
        else if (isType()) value.t = other.value.t;
        else if (isBool()) value.b = other.value.b;
        else if (isSymbol()) value.i = other.value.i;
        else if (isString()) value.s = other.value.s, value.s->inc(), prev.s ? prev.s->dec() : void();
        else if (isArray()) value.a = other.value.a, value.a->inc(), prev.a ? prev.a->dec() : void();
        else if (isUnion()) value.un = other.value.un, value.un->inc(), prev.un ? prev.un->dec() : void();
        else if (isIntersect()) value.in = other.value.in, value.in->inc(), prev.in ? prev.in->dec() : void();
        else if (isFunction()) value.f = other.value.f, value.f->inc(), prev.f ? prev.f->dec() : void();
        else if (isRuntime()) value.rt = other.value.rt;
    }

    Meta::Meta(): _type(nullptr) {
        //
    }

    Meta::Meta(const Type* type): _type(type) {
        //
    }

    Meta::Meta(const Type* type, i64 i): Meta(type) {
        value.i = i;
    }

    Meta::Meta(const Type* type, double d): Meta(type) {
        value.d = d;
    }

    Meta::Meta(const Type* type, uchar c): Meta(type) {
        *(uchar*)(&value.c) = c;
    }

    Meta::Meta(const Type* type, const Type* t): Meta(type) {
        value.t = t;
    }

    Meta::Meta(const Type* type, bool b): Meta(type) {
        value.b = b;
    }

    Meta::Meta(const Type* type, const ustring& s): Meta(type) {
        if (type == STRING)
            value.s = new MetaString(s);
        else if (type == SYMBOL)
            value.i = findSymbol(s);
    }

    Meta::Meta(const Type* type, MetaArray* a): Meta(type) {
        value.a = a;
    }

    Meta::Meta(const Type* type, MetaUnion* un): Meta(type) {
        value.un = un;
    }

    Meta::Meta(const Type* type, MetaIntersect* in): Meta(type) {
        value.in = in;
    }

    Meta::Meta(const Type* type, MetaFunction* f): Meta(type) {
        value.f = f;
    }

    Meta::Meta(const Type* type, Node* rt): Meta(type) {
        value.rt = rt;
    }

    Meta::~Meta() {
        free();
    }

    Meta::Meta(const Meta& other) {
        copy(other);
    }

    Meta& Meta::operator=(const Meta& other) {
        if (this != &other) {
            assign(other);
        }
        return *this;
    }

    const Type* Meta::type() const {
        return _type;
    }

    bool Meta::isVoid() const {
        return _type == VOID;
    }

    bool Meta::isInt() const {
        if (!_type) return false;
        return _type->kind() == Kind::NUMBER 
            && !((NumberType*)_type)->floating();
    }

    i64 Meta::asInt() const {
        return value.i;
    }

    i64& Meta::asInt() {
        return value.i;
    }

    bool Meta::isFloat() const {
        if (!_type) return false;
        return _type->kind() == Kind::NUMBER 
            && ((NumberType*)_type)->floating();
    }

    double Meta::asFloat() const {
        return value.d;
    }

    double& Meta::asFloat() {
        return value.d;
    }

    bool Meta::isChar() const {
        return _type == CHAR;
    }

    uchar Meta::asChar() const {
        return *(uchar*)(&value.c);
    }

    uchar& Meta::asChar() {
        return *(uchar*)(&value.c);
    }

    bool Meta::isType() const {
        return _type == TYPE;
    }

    const Type* Meta::asType() const {
        return value.t;
    }

    const Type*& Meta::asType() {
        return value.t;
    }

    bool Meta::isBool() const {
        return _type == BOOL;
    }

    bool Meta::asBool() const {
        return value.b;
    }

    bool& Meta::asBool() {
        return value.b;
    }

    bool Meta::isSymbol() const {
        return _type == SYMBOL;
    }

    i64 Meta::asSymbol() const {
        return value.i;
    }

    i64& Meta::asSymbol() {
        return value.i;
    }

    bool Meta::isString() const {
        return _type == STRING;
    }

    const ustring& Meta::asString() const {
        return value.s->str();
    }

    ustring& Meta::asString() {
        return value.s->str();
    }

    bool Meta::isArray() const {
        if (!_type) return false;
        return _type->kind() == Kind::ARRAY;
    }

    const MetaArray& Meta::asArray() const {
        return *value.a;
    }

    MetaArray& Meta::asArray() {
        return *value.a;
    }

    bool Meta::isUnion() const {
        if (!_type) return false;
        return _type->kind() == Kind::SUM;
    }

    const MetaUnion& Meta::asUnion() const {
        return *value.un;
    }

    MetaUnion& Meta::asUnion() {
        return *value.un;
    }

    bool Meta::isIntersect() const {
        if (!_type) return false;
        return _type->kind() == Kind::INTERSECT;
    }

    const MetaIntersect& Meta::asIntersect() const {
        return *value.in;
    }

    MetaIntersect& Meta::asIntersect() {
        return *value.in;
    }

    bool Meta::isFunction() const {
        if (!_type) return false;
        return _type->kind() == Kind::FUNCTION;
    }

    const MetaFunction& Meta::asFunction() const {
        return *value.f;
    }

    MetaFunction& Meta::asFunction() {
        return *value.f;
    }

    bool Meta::isRuntime() const {
        if (!_type) return false;
        return _type->kind() == Kind::RUNTIME;
    }

    Node* Meta::asRuntime() const {
        return value.rt;
    }

    Node*& Meta::asRuntime() {
        return value.rt;
    }

    Meta::operator bool() const {
        return _type;
    }

    Meta Meta::clone() const {
        if (isString()) return value.s->clone(*this);
        else if (isArray()) return value.a->clone(*this);
        else if (isUnion()) return value.un->clone(*this);
        else if (isIntersect()) return value.in->clone(*this);
        else if (isFunction()) return value.f->clone(*this);
        return *this;
    }

    void Meta::format(stream& io) const {
        if (!_type) write(io, "<undefined>");
        else if (isVoid()) write(io, "()");
        else if (isInt()) write(io, asInt());
        else if (isFloat()) write(io, asFloat());
        else if (isChar()) write(io, asChar());
        else if (isType()) write(io, asType());
        else if (isBool()) write(io, asBool());
        else if (isSymbol()) write(io, findSymbol(asSymbol()));
        else if (isString()) write(io, asString());
        else if (isArray()) {
            write(io, "[");
            for (u32 i = 0; i < asArray().size(); i ++) {
                write(io, i != 0 ? " " : "", asArray()[i]);
            }
            write(io, "]");
        }
        else if (isUnion())
            write(io, asUnion().value());
        else if (isIntersect()) {
            write(io, "(&");
            for (const Type* t : ((IntersectType*)type())->members()) {
                write(io, " ", asIntersect().as(t));
            }
            write(io, ")");
        }
        else if (isFunction()) write(io, "<function>");
        else if (isRuntime()) write(io, "<unknown>");
    }

    bool Meta::operator==(const Meta& m) const {
        if (_type != m._type) return false;
        if (!_type) return true;
        else if (isVoid()) return true;
        else if (isInt()) return asInt() == m.asInt();
        else if (isFloat()) return asFloat() == m.asFloat();
        else if (isChar()) return asChar() == m.asChar();
        else if (isType()) return asType() == m.asType();
        else if (isBool()) return asBool() == m.asBool();
        else if (isSymbol()) return asSymbol() == m.asSymbol();
        else if (isString()) return asString() == m.asString();
        else if (isArray()) {
            for (u32 i = 0; i < asArray().size(); i ++) {
                if (asArray()[i] != m.asArray()[i]) return false;
            }
        }
        else if (isUnion())
            return m.asUnion().value() == asUnion().value();
        else if (isIntersect()) {
            for (const Type* t : ((IntersectType*)type())->members()) {
                if (asIntersect().as(t) != m.asIntersect().as(t)) return false;
            }
        }
        else if (isFunction()) 
            return asFunction().function() == m.asFunction().function();
        else if (isRuntime())
            return asRuntime() == m.asRuntime();
        return true;
    }

    bool Meta::operator!=(const Meta& m) const {
        return !(*this == m);
    }

    u64 Meta::hash() const {
        u64 h = ::hash(_type);
        if (!_type) return h;
        else if (isVoid()) return h;
        else if (isInt()) return h ^ ::hash(asInt());
        else if (isFloat()) return h ^ ::hash(asFloat());
        else if (isChar()) return h ^ ::hash(asChar());
        else if (isType()) return h ^ ::hash(asType());
        else if (isBool()) return h ^ ::hash(asBool());
        else if (isSymbol()) return h ^ ::hash(asSymbol());
        else if (isString()) return h ^ ::hash(asString());
        else if (isArray()) {
            for (u32 i = 0; i < asArray().size(); i ++) {
                h ^= asArray()[i].hash();
            }
            return h;
        }
        else if (isUnion())
            return h ^ asUnion().value().hash();
        else if (isIntersect()) {
            for (const Type* t : ((IntersectType*)type())->members()) {
                h ^= asIntersect().as(t);
            }
            return h;
        }
        else if (isFunction()) 
            return h ^ ::hash(asFunction().function());
        else if (isRuntime())
            return h ^ ::hash(asRuntime());
        return h;
    }

    ustring Meta::toString() const {
        buffer b;
        format(b);
        ustring s;
        read(b, s);
        return s;
    }

    // MetaRC

    MetaRC::MetaRC(): rc(1) {
        //
    }

    MetaRC::~MetaRC() {
        //
    }
     
    void MetaRC::inc() {
        rc ++;
    }

    void MetaRC::dec() {
        rc --;
        if (rc == 0) delete this;
    }

    // MetaString

    MetaString::MetaString(const ustring& str):
        s(str) {
        //
    }

    const ustring& MetaString::str() const {
        return s;
    }

    ustring& MetaString::str() {
        return s;
    }

    Meta MetaString::clone(const Meta& src) const {
        return Meta(src.type(), str());
    }

    // MetaArray

    MetaArray::MetaArray(const vector<Meta>& values): vals(values) {
        //
    }

    const Meta& MetaArray::operator[](u32 i) const {
        return vals[i];
    }

    Meta& MetaArray::operator[](u32 i) {
        return vals[i];
    }

    const Meta* MetaArray::begin() const {
        return vals.begin();
    }

    const Meta* MetaArray::end() const {
        return vals.end();
    }

    Meta* MetaArray::begin() {
        return vals.begin();
    }

    Meta* MetaArray::end() {
        return vals.end();
    }

    u32 MetaArray::size() const {
        return vals.size();
    }

    Meta MetaArray::clone(const Meta& src) const {
        vector<Meta> copies;
        for (const Meta& m : *this) copies.push(m.clone());
        return Meta(src.type(), new MetaArray(copies));
    }

    // MetaUnion

    MetaUnion::MetaUnion(const Meta& val): real(val) {
        //
    }

    bool MetaUnion::is(const Type* t) const {
        return real.type()->explicitly(t);
    }

    const Meta& MetaUnion::value() const {
        return real;
    }

    Meta& MetaUnion::value() {
        return real;
    }

    Meta MetaUnion::clone(const Meta& src) const {
        return Meta(src.type(), new MetaUnion(real));
    }

    // MetaIntersect

    MetaIntersect::MetaIntersect(const vector<Meta>& values): vals(values) {
        vals.push(Meta());
    }

    const Meta& MetaIntersect::as(const Type* t) const {
        for (const Meta& m : vals) if (m.type() == t) return m;
        return vals.back();
    }

    Meta& MetaIntersect::as(const Type* t) {
        for (Meta& m : vals) if (m.type() == t) return m;
        return vals.back();
    }

    const Meta* MetaIntersect::begin() const {
        return vals.begin();
    }

    const Meta* MetaIntersect::end() const {
        return begin() + size();
    }

    Meta* MetaIntersect::begin() {
        return vals.begin();
    }

    Meta* MetaIntersect::end() {
        return begin() + size();
    }

    u32 MetaIntersect::size() const {
        return size() - 1;
    }

    Meta MetaIntersect::clone(const Meta& src) const {
        vector<Meta> copies;
        for (const Meta& m : vals) if (m) copies.push(m.clone());
        return Meta(src.type(), new MetaIntersect(copies));
    }

    // MetaFunction

    MetaFunction::MetaFunction(Node* function, const vector<u32>& args, Env* local): 
        fn(function), _args(args), _builtin(nullptr), _local(local) {
        //
    }

    MetaFunction::MetaFunction(Builtin builtin): 
        fn(nullptr), _builtin(builtin), _local(nullptr) {
        //
    }

    MetaFunction::~MetaFunction() {
        if (_local) delete _local;
    }

    Node* MetaFunction::function() const {
        return fn;
    }

    Builtin MetaFunction::builtin() const {
        return _builtin;
    }

    Env* MetaFunction::local() const {
        return _local;
    }

    Entry* MetaFunction::arg(u32 i) const {
        return _local->entry(_args[i]);
    }

    Meta MetaFunction::clone(const Meta& src) const {
        return Meta(src.type(), new MetaFunction(fn, _args, _local->fork()));
    }

    // Meta Ops

    bool isNumber(const Type* t) {
        return t->kind() == Kind::NUMBER;
    }
    
    bool isFloat(const Type* t) {
        return isNumber(t) && ((NumberType*)t)->floating();
    }

    bool isRuntime(const Type* t) {
        return t->kind() == Kind::RUNTIME;
    }

    const Type* runtimeType(const Type* t) {
        return ((RuntimeType*)t)->child();
    }

    bool null(const Meta& m) {
        return m.isRuntime() && !m.asRuntime();
    }

    i64 trunc(i64 n, const Type* dest) {
        switch (dest->size()) {
            case 1:
                return i8(n);
            case 2:
                return i16(n);
            case 4:
                return i32(n);
            default:
                return n;
        }
    }

    u64 trunc(u64 n, const Type* dest) {
        switch(dest->size()) {
            case 1:
                return u8(n);
            case 2:
                return u16(n);
            case 4:
                return u32(n);
            default:
                return n;
        }
    }

    double toFloat(const Meta& m) {
        if (m.isFloat()) return m.asFloat();
        else if (m.isInt()) return m.asInt();
        return 0;
    }

    i64 toInt(const Meta& m) {
        if (m.isInt()) return m.asInt();
        else if (m.isFloat()) return i64(m.asFloat());
        return 0;
    }

    double fmod(double l, double r) {
        i64 denom = l / r < 0 ? i64(l / r) - 1 : i64(l / r);
        return l - r * denom;
    }

    Meta add(const Meta& lhs, const Meta& rhs) {
        if (!lhs || !rhs) return Meta();
        const Type* dst = join(lhs.type(), rhs.type());
        if (!dst) return Meta();

        if (isRuntime(dst))
            if (null(lhs) || null(rhs)) return Meta(dst, (Node*)nullptr);

        if (isFloat(dst)) 
            return Meta(dst, toFloat(lhs) + toFloat(rhs));
        else if (isNumber(dst))
            return Meta(dst, trunc(toInt(lhs) + toInt(rhs), dst));
        else if (dst == STRING)
            return Meta(dst, lhs.asString() + rhs.asString());
        return Meta();
    }

    Meta sub(const Meta& lhs, const Meta& rhs) {
        if (!lhs || !rhs) return Meta();
        const Type* dst = join(lhs.type(), rhs.type());
        if (!dst) return Meta();
        if (isFloat(dst)) 
            return Meta(dst, toFloat(lhs) - toFloat(rhs));
        else if (isNumber(dst))
            return Meta(dst, trunc(toInt(lhs) - toInt(rhs), dst));
        return Meta();
    }

    Meta mul(const Meta& lhs, const Meta& rhs) {
        if (!lhs || !rhs) return Meta();
        const Type* dst = join(lhs.type(), rhs.type());
        if (!dst) return Meta();
        if (isFloat(dst)) 
            return Meta(dst, toFloat(lhs) * toFloat(rhs));
        else if (isNumber(dst))
            return Meta(dst, trunc(toInt(lhs) * toInt(rhs), dst));
        return Meta();
    }

    Meta div(const Meta& lhs, const Meta& rhs) {
        if (!lhs || !rhs) return Meta();
        const Type* dst = join(lhs.type(), rhs.type());
        if (!dst) return Meta();
        if (isFloat(dst)) 
            return Meta(dst, toFloat(lhs) / toFloat(rhs));
        else if (isNumber(dst))
            return Meta(dst, trunc(toInt(lhs) / toInt(rhs), dst));
        return Meta();
    }

    Meta mod(const Meta& lhs, const Meta& rhs) {
        if (!lhs || !rhs) return Meta();
        const Type* dst = join(lhs.type(), rhs.type());
        if (!dst) return Meta();
        if (isFloat(dst)) 
            return Meta(dst, fmod(toFloat(lhs), toFloat(rhs)));
        else if (isNumber(dst))
            return Meta(dst, trunc(toInt(lhs) % toInt(rhs), dst));
        return Meta();
    }

    Meta andf(const Meta& lhs, const Meta& rhs) {
        if (!lhs.isBool() || !rhs.isBool()) return Meta();
        return Meta(BOOL, lhs.asBool() && rhs.asBool());
    }

    Meta orf(const Meta& lhs, const Meta& rhs) {
        if (!lhs.isBool() || !rhs.isBool()) return Meta();
        return Meta(BOOL, lhs.asBool() || rhs.asBool());
    }

    Meta xorf(const Meta& lhs, const Meta& rhs) {
        if (!lhs.isBool() || !rhs.isBool()) return Meta();
        return Meta(BOOL, bool(lhs.asBool() ^ rhs.asBool()));
    }

    Meta notf(const Meta& operand) {
        if (!operand.isBool()) return Meta();
        return Meta(BOOL, !operand.asBool());
    }

    Meta equal(const Meta& lhs, const Meta& rhs) {
        if (!lhs || !rhs) return Meta();
        return Meta(BOOL, lhs == rhs);
    }

    Meta inequal(const Meta& lhs, const Meta& rhs) {
        if (!lhs || !rhs) return Meta();
        return Meta(BOOL, lhs != rhs);
    }

    Meta less(const Meta& lhs, const Meta& rhs) {
        if (!lhs || !rhs) return Meta();
        const Type* dst = join(lhs.type(), rhs.type());
        if (!dst) return Meta();
        if (isFloat(dst)) 
            return Meta(BOOL, toFloat(lhs) < toFloat(rhs));
        else if (isNumber(dst))
            return Meta(BOOL, toInt(lhs) < toInt(rhs));
        else if (dst == STRING)
            return Meta(BOOL, lhs.asString() < rhs.asString());
        return Meta();
    }

    Meta lessequal(const Meta& lhs, const Meta& rhs) {
        if (!lhs || !rhs) return Meta();
        const Type* dst = join(lhs.type(), rhs.type());
        if (!dst) return Meta();
        if (isFloat(dst)) 
            return Meta(BOOL, toFloat(lhs) <= toFloat(rhs));
        else if (isNumber(dst))
            return Meta(BOOL, toInt(lhs) <= toInt(rhs));
        else if (dst == STRING)
            return Meta(BOOL, lhs.asString() <= rhs.asString());
        return Meta();
    }

    Meta greater(const Meta& lhs, const Meta& rhs) {
        if (!lhs || !rhs) return Meta();
        const Type* dst = join(lhs.type(), rhs.type());
        if (!dst) return Meta();
        if (isFloat(dst)) 
            return Meta(BOOL, toFloat(lhs) > toFloat(rhs));
        else if (isNumber(dst))
            return Meta(BOOL, toInt(lhs) > toInt(rhs));
        else if (dst == STRING)
            return Meta(BOOL, lhs.asString() > rhs.asString());
        return Meta();
    }

    Meta greaterequal(const Meta& lhs, const Meta& rhs) {
        if (!lhs || !rhs) return Meta();
        const Type* dst = join(lhs.type(), rhs.type());
        if (!dst) return Meta();
        if (isFloat(dst)) 
            return Meta(BOOL, toFloat(lhs) >= toFloat(rhs));
        else if (isNumber(dst))
            return Meta(BOOL, toInt(lhs) >= toInt(rhs));
        else if (dst == STRING)
            return Meta(BOOL, lhs.asString() >= rhs.asString());
        return Meta();
    }

    Meta unionf(const Meta& lhs, const Meta& rhs) {
        return Meta();
    }

    Meta intersect(const Meta& lhs, const Meta& rhs) {
        return Meta();
    }

    void assign(Meta& lhs, const Meta& rhs) {
        lhs = rhs;
    }

    Meta cast(const Meta& lhs, const Type* dst) {
        if (!lhs || !dst) return Meta();
        if (!lhs.type()->explicitly(dst)) return Meta();
        if (isFloat(dst))
            return Meta(dst, toFloat(lhs));
        else if (isNumber(dst))
            return Meta(dst, toInt(lhs));
        return Meta();
    }
}

template<>
u64 hash(const basil::Meta& m) {
    return m.hash();
}

void write(stream& io, const basil::Meta& m) {
    m.format(io);
}