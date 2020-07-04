#ifndef BASIL_AST_H
#define BASIL_AST_H

#include "defs.h"
#include "utf8.h"
#include "meta.h"

namespace basil {
    class Node {
        u32 _line, _column;
    public:
        Node(u32 line, u32 column);
        virtual ~Node();

        u32 line() const;
        u32 column() const;
        virtual Meta eval(Env* env) = 0;
    };

    class Int : public Node {
        i64 _value;
    public:
        Int(i64 value, u32 line, u32 column);

        virtual Meta eval(Env* env) override;
    };

    class Float : public Node {
        double _value;
    public:
        Float(double value, u32 line, u32 column);

        virtual Meta eval(Env* env) override;
    };

    class String : public Node {
        ustring _value;
    public:
        String(const ustring& value, u32 line, u32 column);

        virtual Meta eval(Env* env) override;
    };

    class Char : public Node {
        uchar _value;
    public:
        Char(uchar value, u32 line, u32 column);

        virtual Meta eval(Env* env) override;
    };

    class Boolean : public Node {
        bool _value;
    public:
        Boolean(const ustring& value, u32 line, u32 column);

        virtual Meta eval(Env* env) override;
    };

    class Variable : public Node {
        ustring _name;
    public:
        Variable(const ustring& name, u32 line, u32 column);

        virtual Meta eval(Env* env) override;
    };

    class Constant : public Node {
        Meta _value;
    public:
        Constant(const Meta& value, u32 line, u32 column);

        virtual Meta eval(Env* env) override;
    };

    class Quote : public Node {
        Term* _term;
    public:
        Quote(Term* term, u32 line, u32 column);

        virtual Meta eval(Env* env) override;
    };

    class Define : public Node {
        Node* _type;
        vector<ustring> _names;
        Node* _init;
    public:
        Define(Node* _type, const vector<ustring>& names, Node* init, u32 line, u32 column);
        Define(const vector<ustring>& names, Node* init, u32 line, u32 column);
        ~Define();

        virtual Meta eval(Env* env) override;
    };

    class Do : public Node {
        vector<Node*> _body;
    public:
        Do(const vector<Node*>& body);
        ~Do();

        virtual Meta eval(Env* env) override;
    };

    class Lambda : public Node {
        Env* _local;
        Node* _type;
        vector<Node*> _args;
        Node* _body;
    public:
        Lambda(Node* type, const vector<Node*>& args, Node* body, u32 line, u32 column);
        Lambda(const vector<Node*>& args, Node* body, u32 line, u32 column);
        ~Lambda();

        virtual Meta eval(Env* env) override;
    };

    class Call : public Node {
        Node* _func;
        vector<Node*> _args;
    public:
        Call(Node* func, const vector<Node*>& args, u32 line, u32 column);
        ~Call();

        virtual Meta eval(Env* env) override;
    };

    class Add : public Node {
        vector<Node*> _params;
    public:
        Add(const vector<Node*>& params, u32 line, u32 column);
        ~Add();

        virtual Meta eval(Env* env) override;
    };

    class Subtract : public Node {
        vector<Node*> _params;
    public:
        Subtract(const vector<Node*>& params, u32 line, u32 column);
        ~Subtract();

        virtual Meta eval(Env* env) override;
    };

    class Multiply : public Node {
        vector<Node*> _params;
    public:
        Multiply(const vector<Node*>& params, u32 line, u32 column);
        ~Multiply();

        virtual Meta eval(Env* env) override;
    };

    class Divide : public Node {
        vector<Node*> _params;
    public:
        Divide(const vector<Node*>& params, u32 line, u32 column);
        ~Divide();

        virtual Meta eval(Env* env) override;
    };
}

#endif