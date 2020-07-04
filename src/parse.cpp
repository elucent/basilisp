#include "parse.h"
#include "io.h"
#include "lex.h"
#include "meta.h"
#include "type.h"
#include "errors.h"
#include "builtin.h"

namespace basil {
    Term::Term(u32 line, u32 column):
        _line(line), _column(column) {
        //
    }

    Term::~Term() {
        //
    }

    u32 Term::line() const {
        return _line;
    }

    u32 Term::column() const {
        return _column;
    }

    BlockTerm::BlockTerm(const vector<Term*> terms, u32 line, u32 column):
        Term(line, column), _terms(terms) {
        //
    }

    BlockTerm::~BlockTerm() {
        for (Term* t : _terms) delete t;
    }

    const vector<Term*>& BlockTerm::terms() const {
        return _terms;
    }

    Node* BlockTerm::eval(Env* env) const {
        Node* n = _terms[0]->eval(env);
        Meta m = n->eval(env);
        if (m.isType())
            return declare(env, n, this);
        else if (m.isFunction() && m.asFunction().builtin())
            return m.asFunction().builtin()(env, n, this);
        else if (m.isFunction())
            return call(env, n, this); // todo: call
        err(PHASE_TYPE, _terms[0]->line(), _terms[0]->column(),
            "First term in block is not a type or function.");
        delete n;
        return nullptr;
    }

    Meta BlockTerm::quote() const {
        vector<Meta> metas;
        set<const Type*> types;
        for (Term* t : _terms) 
            metas.push(t->quote()), types.insert(metas.back().type());
        const Type* type = types.size() == 1 ? *types.begin() : find<SumType>(types);
        return Meta(find<ArrayType>(type, metas.size()), new MetaArray(metas));
    }

    void BlockTerm::format(stream& io) const {
        write(io, "(");
        for (Term* t : _terms) {
            t->format(io);
            if (t != _terms[_terms.size() - 1]) write(io, " ");
        }
        write(io, ")");
    }

    IntTerm::IntTerm(i64 value, u32 line, u32 column):
        Term(line, column), _value(value) {
        //
    }

    Node* IntTerm::eval(Env* env) const {
        return new Int(_value, line(), column());
    }

    Meta IntTerm::quote() const {
        return Meta(INT, _value);
    }

    void IntTerm::format(stream& io) const {
        write(io, _value);
    }

    FloatTerm::FloatTerm(double value, u32 line, u32 column):
        Term(line, column), _value(value) {
        //
    }

    Node* FloatTerm::eval(Env* env) const {
        return new Float(_value, line(), column());
    }

    Meta FloatTerm::quote() const {
        return Meta(FLOAT, _value);
    }

    void FloatTerm::format(stream& io) const {
        write(io, _value);
    }

    CharTerm::CharTerm(uchar value, u32 line, u32 column):
        Term(line, column), _value(value) {
        //
    }

    Node* CharTerm::eval(Env* env) const {
        return new Char(_value, line(), column());
    }

    Meta CharTerm::quote() const {
        return Meta(CHAR, _value);
    }

    void CharTerm::format(stream& io) const {
        write(io, '\'', escape(ustring() + _value), '\'');
    }

    StringTerm::StringTerm(const ustring& value, u32 line, u32 column):
        Term(line, column), _value(value) {
        //
    }

    Node* StringTerm::eval(Env* env) const {
        return new String(_value, line(), column());
    }

    Meta StringTerm::quote() const {
        return Meta(STRING, _value);
    }

    void StringTerm::format(stream& io) const {
        write(io, '"', escape(_value), '"');
    }

    VariableTerm::VariableTerm(const ustring& name, u32 line, u32 column):
        Term(line, column), _name(name) {
        //
    }

    Node* VariableTerm::eval(Env* env) const {
        return new Variable(_name, line(), column());
    }

    Meta VariableTerm::quote() const {
        return Meta(SYMBOL, _name);
    }

    void VariableTerm::format(stream& io) const {
        write(io, _name);
    }

    Term* parse(TokenView& view);

    static i64 stringToInt(const ustring& s) {
        buffer b;
        i64 i;
        write(b, s);
        read(b, i);
        return i;
    }

    static double stringToFloat(const ustring& s) {
        buffer b;
        double d;
        write(b, s);
        read(b, d);
        return d;
    }

    Term* parseArray(TokenView& view) {
        u32 line = view.peek().line, column = view.peek().column;
        view.read();
        vector<Term*> contents = { new VariableTerm("array", line, column) };
        while (view.peek().id != T_RBRACK) {
            if (view.peek().id == T_NONE) {
                err(PHASE_PARSE, view.peek().line, view.peek().column,
                    "Unexpected end of file.");
                for (Term* t : contents) delete t;
                return nullptr;
            }
            if (Term* t = parse(view))
                contents.push(t);
        }
        view.read();
        return new BlockTerm(contents, line, column);
    }

    Term* parseBlock(TokenView& view) {
        u32 line = view.peek().line, column = view.peek().column;
        view.read();
        vector<Term*> contents;
        while (view.peek().id != T_RPAREN) {
            if (view.peek().id == T_NONE) {
                err(PHASE_PARSE, view.peek().line, view.peek().column,
                    "Unexpected end of file.");
                for (Term* t : contents) delete t;
                return nullptr;
            }
            if (Term* t = parse(view))
                contents.push(t);
        }
        view.read();
        return new BlockTerm(contents, line, column);
    }

    Term* parse(TokenView& view) {
        const Token& t = view.peek();
        switch (t.id) {
            case T_INT:
                view.read();
                return new IntTerm(stringToInt(t.name), t.line, t.column);
            case T_FLOAT:
                view.read();
                return new FloatTerm(stringToFloat(t.name), t.line, t.column);
            case T_STRING:
                view.read();
                return new StringTerm(t.name, t.line, t.column);
            case T_CHAR:
                view.read();
                return new CharTerm(t.name[0], t.line, t.column);
            case T_IDENT:
                view.read();
                return new VariableTerm(t.name, t.line, t.column);
            case T_QUOTE:
                view.read();
                return new BlockTerm({
                    new VariableTerm("quote", t.line, t.column),
                    parse(view)
                }, t.line, t.column);
            case T_LPAREN:
                return parseBlock(view);
            case T_LBRACK:
                return parseArray(view);
            default:
                err(PHASE_PARSE, t.line, t.column,
                    "Unexpected token '", t.name, "'.");
                return nullptr;
        }
    }
}

void write(stream& io, const basil::Term* term) {
    term->format(io);
}

void write(stream& io, basil::Term* term) {
    term->format(io);
}