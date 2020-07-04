#include "builtin.h"
#include "ast.h"
#include "env.h"
#include "parse.h"
#include "type.h"
#include "meta.h"
#include "errors.h"

namespace basil {
    Node* define(Env* env, Node* func, const BlockTerm* term) {
        delete func;
        
        u32 i = 1;
        vector<ustring> names;
        BlockTerm* fnargs = nullptr;
        while (i < term->terms().size()) {
            Meta q = term->terms()[i]->quote();

            // declare a function if we encounter a block before the end
            if (q.isArray() && i + 1 < term->terms().size()) {
                fnargs = (BlockTerm*)term->terms()[i];
                ++ i;
                break;
            }

            // stop declaring new variables when we find a value or
            // an existing variable
            if (!q.isSymbol() || env->lookup(findSymbol(q.asSymbol())))
                break;
            names.push(findSymbol(q.asSymbol()));
            ++ i;
        }

        if (names.size() == 0) {
            err(PHASE_TYPE, term->line(), term->column(),
                "No variable names provided in definition.");
            return nullptr;
        }

        if (fnargs) {
            Node* args = fnargs->eval(env);

            vector<Node*> bodyvals;
            for (; i < term->terms().size(); i ++)
                bodyvals.push(term->terms()[i]->eval(env));
            Node* body = new Do(bodyvals);

            return new Define(names, 
                new Lambda({ args }, body, term->line(), term->column()),
                term->line(), term->column());
        }

        if (i == term->terms().size()) {
            err(PHASE_TYPE, term->terms()[i - 1]->line(), term->terms()[i - 1]->column(),
                "No initial value provided in variable declaration.");
            return nullptr;
        }
        else if (i < term->terms().size() - 1) {
            err(PHASE_TYPE, term->terms()[i]->line(), term->terms()[i]->column(),
                "More than one initial value provided in variable declaration.");
            return nullptr;
        }

        Node* init = term->terms()[i]->eval(env);
        return new Define(names, init, term->line(), term->column());
    }

    Node* declare(Env* env, Node* type, const BlockTerm* term) {
        Meta m = type->eval(env);
        if (!m.isType()) {
            err(PHASE_TYPE, term->terms()[0]->line(), term->terms()[0]->column(),
                "Could not resolve type in declaration.");
            delete type;
            return nullptr;
        }

        u32 i = 1;
        vector<ustring> names;
        BlockTerm* fnargs = nullptr;
        while (i < term->terms().size()) {
            Meta q = term->terms()[i]->quote();

            // declare a function if we encounter a block before the end
            if (q.isArray() && i + 1 < term->terms().size()) {
                fnargs = (BlockTerm*)term->terms()[i];
                ++ i;
                break;
            }

            // stop declaring new variables when we find a value or
            // an existing variable
            if (!q.isSymbol() || env->lookup(findSymbol(q.asSymbol())))
                break;
            names.push(findSymbol(q.asSymbol()));
            ++ i;
        }

        if (names.size() == 0) {
            err(PHASE_TYPE, term->line(), term->column(),
                "No variable names provided in definition.");
            return nullptr;
        }

        if (fnargs) {
            Node* args = fnargs->eval(env);

            vector<Node*> bodyvals;
            for (; i < term->terms().size(); i ++)
                bodyvals.push(term->terms()[i]->eval(env));
            Node* body = new Do(bodyvals);

            return new Define(names, 
                new Lambda(type, { args }, body, term->line(), term->column()),
                term->line(), term->column());
        }

        if (i < term->terms().size() - 1) {
            err(PHASE_TYPE, term->terms()[i + 1]->line(), term->terms()[i + 1]->column(),
                "More than one initial value provided in variable declaration.");
            delete type;
            return nullptr;
        }

        Node* init = i < term->terms().size() ? term->terms()[i]->eval(env) : nullptr;
        return new Define(type, names, init, term->line(), term->column());
    }

    Node* lambda(Env* env, Node* func, const BlockTerm* term) {
        delete func;

        if (term->terms().size() < 3) {
            err(PHASE_TYPE, term->line(), term->column(),
                "Not enough arguments in lambda expression: expected at least 3, ",
                "found ", term->terms().size() - 1);
            return nullptr;
        }

        Meta qargs = term->terms()[1]->quote();
        if (!qargs.isArray()) {
            err(PHASE_TYPE, term->terms()[1]->line(), term->terms()[1]->column(),
                "Expected argument block in lambda expression.");
            return nullptr;
        }

        Node* args = term->terms()[1]->eval(env);

        vector<Node*> bodyvals;
        for (u32 i = 2; i < term->terms().size(); i ++)
            bodyvals.push(term->terms()[i]->eval(env));
        Node* body = new Do(bodyvals);

        return new Lambda({ args }, body, term->line(), term->column());
    }

    Node* call(Env* env, Node* func, const BlockTerm* term) {
        vector<Node*> args;
        for (u32 i = 1; i < term->terms().size(); i ++) 
            args.push(term->terms()[i]->eval(env));
        return new Call(func, args, term->line(), term->column());
    }

    Node* quote(Env* env, Node* func, const BlockTerm* term) {
        delete func;

        return new Quote(term->terms()[1], term->line(), term->column());
    }

    Node* doBlock(Env* env, Node* func, const BlockTerm* term) {
        delete func;

        vector<Node*> body;
        for (u32 i = 1; i < term->terms().size(); i ++) {
            body.push(term->terms()[i]->eval(env));
        }

        if (!body.size()) {
            err(PHASE_TYPE, term->line(), term->column(),
                "No body provided to do-expression.");
            return nullptr;
        }

        return new Do(body);
    }

    Node* add(Env* env, Node* func, const BlockTerm* term) {
        delete func;

        vector<Node*> params;
        for (u32 i = 1; i < term->terms().size(); i ++)
            params.push(term->terms()[i]->eval(env));
        
        if (params.size() == 0) {
            err(PHASE_TYPE, term->line(), term->column(),
                "No parameters provided to built-in function '+'.");
            return nullptr;
        }

        return new Add(params, term->line(), term->column());
    }

    Node* subtract(Env* env, Node* func, const BlockTerm* term) {
        delete func;
        vector<Node*> params;
        for (u32 i = 1; i < term->terms().size(); i ++)
            params.push(term->terms()[i]->eval(env));
        
        if (params.size() == 0) {
            err(PHASE_TYPE, term->line(), term->column(),
                "No parameters provided to built-in function '-'.");
            return nullptr;
        }

        return new Subtract(params, term->line(), term->column());
    }

    Node* multiply(Env* env, Node* func, const BlockTerm* term) {
        delete func;
        vector<Node*> params;
        for (u32 i = 1; i < term->terms().size(); i ++)
            params.push(term->terms()[i]->eval(env));
        
        if (params.size() == 0) {
            err(PHASE_TYPE, term->line(), term->column(),
                "No parameters provided to built-in function '*'.");
            return nullptr;
        }

        return new Multiply(params, term->line(), term->column());
    }

    Node* divide(Env* env, Node* func, const BlockTerm* term) {
        delete func;
        vector<Node*> params;
        for (u32 i = 1; i < term->terms().size(); i ++)
            params.push(term->terms()[i]->eval(env));
        
        if (params.size() == 0) {
            err(PHASE_TYPE, term->line(), term->column(),
                "No parameters provided to built-in function '/'.");
            return nullptr;
        }

        return new Divide(params, term->line(), term->column());
    }
}