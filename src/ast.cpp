#include "ast.h"
#include "parse.h"
#include "type.h"
#include "meta.h"
#include "env.h"
#include "errors.h"

namespace basil {

    // Node
    
    Node::Node(u32 line, u32 column):
        _line(line), _column(column) {
        //
    }

    Node::~Node() {
        //
    }
    
    u32 Node::line() const {
        return _line;
    }

    u32 Node::column() const {
        return _column;
    }

    // Int

    Int::Int(i64 value, u32 line, u32 column):
        Node(line, column), _value(value) {
        //
    }

    Meta Int::eval(Env* env) {
        return Meta(INT, _value);
    }

    // Float

    Float::Float(double value, u32 line, u32 column):
        Node(line, column), _value(value) {
        //
    }

    Meta Float::eval(Env* env) {
        return Meta(FLOAT, _value);
    }

    // String

    String::String(const ustring& value, u32 line, u32 column):
        Node(line, column), _value(value) {
        //
    }

    Meta String::eval(Env* env) {
        return Meta(STRING, _value);
    }

    // Char

    Char::Char(uchar value, u32 line, u32 column):
        Node(line, column), _value(value) {
        //
    }

    Meta Char::eval(Env* env) {
        return Meta(CHAR, _value);
    }

    // Variable

    Variable::Variable(const ustring& name, u32 line, u32 column):
        Node(line, column), _name(name) {
        //
    }

    Meta Variable::eval(Env* env) {
        Entry* entry = env->lookup(_name);
        if (entry) return entry->meta;

        err(PHASE_TYPE, line(), column(),
            "Undefined variable '", _name, "'.");
        return Meta();
    }

    // Constant

    Constant::Constant(const Meta& value, u32 line, u32 column):
        Node(line, column), _value(value) {
        //
    }

    Meta Constant::eval(Env* env) {
        return _value;
    }

    // Quote

    Quote::Quote(Term* term, u32 line, u32 column):
        Node(line, column), _term(term) {
        //
    }

    Meta Quote::eval(Env* env) {
        return _term->quote();
    }

    // Define

    Define::Define(Node* type, const vector<ustring>& names, Node* init, u32 line, u32 column):
        Node(line, column), _type(type), _names(names), _init(init) {
        //
    }

    Define::Define(const vector<ustring>& names, Node* init, u32 line, u32 column):
        Node(line, column), _type(nullptr), _names(names), _init(init) {
        //
    }

    Define::~Define() {
        if (_type) delete _type;
        if (_init) delete _init;
    }

    Meta Define::eval(Env* env) {
        Meta initval = _init ? _init->eval(env) : Meta();
        const Type* type = nullptr;;
        if (_type) {
            Meta typeval = _type->eval(env);
            if (!typeval.isType()) {
                err(PHASE_TYPE, _type->line(), _type->column(),
                    "Could not resolve definition type - expected '", TYPE, "' ",
                    "but found '", typeval.type(), "'.");
                return Meta();
            }
            type = typeval.asType();
        }
        else if (initval) type = initval.type();
        else {
            err(PHASE_TYPE, line(), column(),
                "Neither an explicit type nor initializer were provided in definition.");
            return Meta();
        }

        if (initval && !initval.type()->implicitly(type)) {
            err(PHASE_TYPE, _init->line(), _init->column(),
                "Could not convert initial value of type '", initval.type(),
                "' to definition type '", type, "'.");
            return Meta();
        }

        if (!initval) initval = Meta(find<RuntimeType>(type), (Node*)nullptr);

        for (const ustring& name : _names)
            env->enter(name, initval);

        return initval;
    }

    // Do        
    
    Do::Do(const vector<Node*>& body):
        Node(body[0]->line(), body[0]->column()), _body(body) {
        //
    }

    Do::~Do() {
        for (Node* n : _body) delete n;
    }

    Meta Do::eval(Env* env) {
        for (Node* n : _body) {
            if (n == _body.back())
                return n->eval(env);
            else n->eval(env);
        }
        return Meta();
    }

    // Lambda

    Lambda::Lambda(Node* type, const vector<Node*>& args, Node* body, u32 line, u32 column):
        Node(line, column), _type(type), _args(args), _body(body), _local(nullptr) {
        //
    }

    Lambda::Lambda(const vector<Node*>& args, Node* body, u32 line, u32 column):
        Node(line, column), _type(nullptr), _args(args), _body(body), _local(nullptr) {
        //
    }

    Lambda::~Lambda() {
        for (Node* n : _args) delete n;
        if (_type) delete _type;
        delete _body;
    }

    Meta Lambda::eval(Env* env) {
        if (!_local) {
            _local = new Env();
            _local->setParent(env);
            for (Node* n : _args) n->eval(_local);
        }

        vector<const Type*> args;
        for (auto& entry : _local->entries()) {
            const Meta& m = entry.second.meta;

            // unbound, runtime-determined value signals an argument
            if (m.type()->kind() == Kind::RUNTIME && m.asRuntime() == nullptr)
                args.push(((RuntimeType*)m.type())->child()); // erase runtime attribute
        }
        
        const Type* rettype = nullptr;
        if (_type) {
            Meta typeval = _type->eval(env);
            if (!typeval.isType()) {
                err(PHASE_TYPE, _type->line(), _type->column(),
                    "Could not resolve return type - expected '", TYPE, "' ",
                    "but found '", typeval.type(), "'.");
                return Meta();
            }
            rettype = typeval.asType();
        }
        else {
            Meta m = _body->eval(_local);
            if (!m) {
                err(PHASE_TYPE, _body->line(), _body->column(),
                    "Could not infer return type from function body.");
                return Meta();
            }
            rettype = m.type();
        }

        if (rettype->kind() == Kind::RUNTIME)
            rettype = ((RuntimeType*)rettype)->child();

        Env* valenv = _local->fork();
        valenv->setParent(env);
        vector<u32> valargs;
        
        for (u32 i = 0; i < valenv->entries().size(); i ++) {
            const Meta& m = valenv->entry(i)->meta;

            // unbound, runtime-determined value signals an argument
            if (m.type()->kind() != Kind::RUNTIME || m.asRuntime()) break;
            
            valargs.push(i);
        }

        return Meta(find<FunctionType>(args, rettype), 
            new MetaFunction(_body, valargs, valenv));
    }

    // Call

    Call::Call(Node* func, const vector<Node*>& args, u32 line, u32 column):
        Node(line, column), _func(func), _args(args) {
        //
    }

    Call::~Call() {
        delete _func;
        for (Node* n : _args) delete n;
    }

    Meta Call::eval(Env* env) {
        Meta m = _func->eval(env);
        if (!m.isFunction()) {
            err(PHASE_TYPE, _func->line(), _func->column(),
                "Could not resolve function to be called.");
            return Meta();
        }
        MetaFunction& f = m.asFunction();
        FunctionType* ft = (FunctionType*)m.type();

        if (ft->args().size() != _args.size()) {
            err(PHASE_TYPE, line(), column(),
                "Incorrect number of arguments: expected ", ft->args().size(),
                ", found ", _args.size(), ".");
            return Meta();
        }

        for (u32 i = 0; i < ft->args().size(); i ++) {
            Meta m = _args[i]->eval(env);
            if (!m.type()->implicitly(ft->args()[i])) {
                err(PHASE_TYPE, _args[i]->line(), _args[i]->column(),
                    "Incorrect argument type: expected '",
                    ft->args()[i], "', but found '", m.type(), "'.");
                return Meta();
            }
            f.arg(i)->meta = m;
        }
        Meta result = f.function()->eval(f.local());

        return result;
    }

    // Add

    Add::Add(const vector<Node*>& params, u32 line, u32 column):
        Node(line, column), _params(params) {
        //
    }

    Add::~Add() {
        for (Node* n : _params) delete n;
    }

    Meta Add::eval(Env* env) {
        Meta m = _params[0]->eval(env);
        for (u32 i = 1; i < _params.size(); i ++)
            m = add(m, _params[i]->eval(env));
        return m;
    }

    // Subtract

    Subtract::Subtract(const vector<Node*>& params, u32 line, u32 column):
        Node(line, column), _params(params) {
        //
    }

    Subtract::~Subtract() {
        for (Node* n : _params) delete n;
    }

    Meta Subtract::eval(Env* env) {
        Meta m = _params[0]->eval(env);
        if (_params.size() == 1) // negate
            return m.isInt() 
                ? sub(Meta(INT, i64(0)), m) 
                : sub(Meta(FLOAT, 0.0), m);
        for (u32 i = 1; i < _params.size(); i ++)
            m = sub(m, _params[i]->eval(env));
        return m;
    }

    // Multiply

    Multiply::Multiply(const vector<Node*>& params, u32 line, u32 column):
        Node(line, column), _params(params) {
        //
    }

    Multiply::~Multiply() {
        for (Node* n : _params) delete n;
    }

    Meta Multiply::eval(Env* env) {
        Meta m = _params[0]->eval(env);
        for (u32 i = 1; i < _params.size(); i ++)
            m = mul(m, _params[i]->eval(env));
        return m;
    }

    // Divide

    Divide::Divide(const vector<Node*>& params, u32 line, u32 column):
        Node(line, column), _params(params) {
        //
    }

    Divide::~Divide() {
        for (Node* n : _params) delete n;
    }

    Meta Divide::eval(Env* env) {
        Meta m = _params[0]->eval(env);
        if (_params.size() == 1) // negate
            return m.isInt() 
                ? div(Meta(INT, i64(1)), m) 
                : div(Meta(FLOAT, 1.0), m);
        for (u32 i = 1; i < _params.size(); i ++)
            m = div(m, _params[i]->eval(env));
        return m;
    }
}