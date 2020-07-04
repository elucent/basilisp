#ifndef BASIL_BUILTIN_H
#define BASIL_BUILTIN_H

#include "defs.h"
#include "ast.h"

namespace basil {
    Node* define(Env* env, Node* func, const BlockTerm* term);
    Node* declare(Env* env, Node* type, const BlockTerm* term);
    Node* lambda(Env* env, Node* func, const BlockTerm* term);
    Node* call(Env* env, Node* type, const BlockTerm* term);
    Node* quote(Env* env, Node* func, const BlockTerm* term);
    Node* doBlock(Env* env, Node* func, const BlockTerm* term);
    Node* add(Env* env, Node* func, const BlockTerm* term);
    Node* subtract(Env* env, Node* func, const BlockTerm* term);
    Node* multiply(Env* env, Node* func, const BlockTerm* term);
    Node* divide(Env* env, Node* func, const BlockTerm* term);
    Node* modulo(Env* env, Node* func, const BlockTerm* term);
}

#endif