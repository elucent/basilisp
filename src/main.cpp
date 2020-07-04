#include "source.h"
#include "lex.h"
#include "errors.h"
#include "parse.h"
#include "type.h"
#include "meta.h"
#include "env.h"
#include "builtin.h"

using namespace basil;

Env* createRootEnv() {
    Env* env = new Env();
    env->enter("int", Meta(TYPE, INT));
    env->enter("float", Meta(TYPE, FLOAT));
    env->enter("type", Meta(TYPE, TYPE));

    const Type* builtinfn = find<FunctionType>(vector<const Type*>{ANY}, ANY);
    env->enter("let", Meta(builtinfn, new MetaFunction(define)));
    env->enter("lambda", Meta(builtinfn, new MetaFunction(lambda)));
    env->enter("+", Meta(builtinfn, new MetaFunction(add)));
    env->enter("-", Meta(builtinfn, new MetaFunction(subtract)));
    env->enter("*", Meta(builtinfn, new MetaFunction(multiply)));
    env->enter("/", Meta(builtinfn, new MetaFunction(divide)));
    env->enter("quote", Meta(builtinfn, new MetaFunction(quote)));
    env->enter("do", Meta(builtinfn, new MetaFunction(doBlock)));
    return env;
}

int repl() {
    Source src;
    useSource(&src);

    Env* root = createRootEnv();
    Env* global = new Env();
    global->setParent(root);

    while (true) {
        vector<Token> tokens;

        print("? ");
        Source::View view = src.expand(_stdin);
        while (view.peek()) {
            if (Token token = lex(view))
                tokens.push(token);
            if (countErrors()) {
                printErrors(_stdout);
                return 1;
            }
        }

        vector<Term*> terms;
        TokenView tview(tokens);
        while (tview.peek()) {
            if (Term* term = parse(tview))
                terms.push(term);
            if (countErrors()) {
                printErrors(_stdout);
                return 1;
            }
        }

        vector<Node*> nodes;
        for (Term* t : terms) {
            if (Node* n = t->eval(global)) nodes.push(n);
            if (countErrors()) {
                printErrors(_stdout);
                return 1;
            }
        }

        println("");
        for (Node* n : nodes) {
            Meta m = n->eval(global);
            if (countErrors()) {
                printErrors(_stdout);
            }
            else if (m) println(m, " : ", m.type());
        }
        println("");
        if (countErrors()) return 1;

        for (Term* t : terms) delete t;
    }

    delete global;
    delete root;

    return 0;
}

int compile(const char* path) {
    Source src(path);
    useSource(&src);

    Env* root = createRootEnv();
    Env* global = new Env();
    global->setParent(root);

    vector<Token> tokens;
    Source::View view = src.view();
    while (view.peek()) {
        if (Token token = lex(view))
            tokens.push(token);
        if (countErrors()) {
            printErrors(_stdout);
            return 1;
        }
    }

    vector<Term*> terms;
    TokenView tview(tokens);
    while (tview.peek()) {
        if (Term* term = parse(tview))
            terms.push(term);
        if (countErrors()) {
            printErrors(_stdout);
            return 1;
        }
    }

    vector<Node*> nodes;
    for (Term* t : terms) {
        if (Node* n = t->eval(global)) nodes.push(n), n->eval(global);
        if (countErrors()) {
            printErrors(_stdout);
            return 1;
        }
    }

    for (Node* n : nodes) {
        Meta m = n->eval(global);
        if (countErrors()) {
            printErrors(_stdout);
        }
        else println(m, " : ", m.type());
    }
    if (countErrors()) return 1;

    for (Term* t : terms) delete t;
    for (Node* n : nodes) delete n;

    delete global;
    delete root;

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) return repl();
    else return compile(argv[1]);
}