#ifndef BASIL_PARSE_H
#define BASIL_PARSE_H

#include "defs.h"
#include "vec.h"
#include "utf8.h"
#include "ast.h"

namespace basil {
    class Term {
        u32 _line, _column;
    public:
        Term(u32 line, u32 column);
        virtual ~Term();

        u32 line() const;
        u32 column() const;
        virtual Node* eval(Env* env) const = 0;
        virtual Meta quote() const = 0;
        virtual void format(stream& io) const = 0;
    };

    class BlockTerm : public Term {
        vector<Term*> _terms;
    public:
        BlockTerm(const vector<Term*> terms, u32 line, u32 column);
        ~BlockTerm();

        const vector<Term*>& terms() const;
        Node* eval(Env* env) const override;
        Meta quote() const override;
        void format(stream& io) const override;
    };

    class IntTerm : public Term {
        i64 _value;
    public:
        IntTerm(i64 value, u32 line, u32 column);

        Node* eval(Env* env) const override;
        Meta quote() const override;
        void format(stream& io) const override;
    };

    class FloatTerm : public Term {
        double _value;
    public:
        FloatTerm(double value, u32 line, u32 column);

        Node* eval(Env* env) const override;
        Meta quote() const override;
        void format(stream& io) const override;
    };

    class CharTerm : public Term {
        uchar _value;
    public:
        CharTerm(uchar value, u32 line, u32 column);

        Node* eval(Env* env) const override;
        Meta quote() const override;
        void format(stream& io) const override;
    };

    class StringTerm : public Term {
        ustring _value;
    public:
        StringTerm(const ustring& value, u32 line, u32 column);

        Node* eval(Env* env) const override;
        Meta quote() const override;
        void format(stream& io) const override;
    };

    class VariableTerm : public Term {
        ustring _name;
    public:
        VariableTerm(const ustring& name, u32 line, u32 column);

        Node* eval(Env* env) const override;
        Meta quote() const override;
        void format(stream& io) const override;
    };

    Term* parse(TokenView& view);
}

void write(stream& io, const basil::Term* term);
void write(stream& io, basil::Term* term);

#endif