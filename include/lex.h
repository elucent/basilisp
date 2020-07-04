#ifndef BASIL_LEX_H
#define BASIL_LEX_H

#include "source.h"
#include "utf8.h"

namespace basil {
    constexpr u32
        T_NONE = 0,
        T_INT = 1,     // [0-9]+
        T_FLOAT = 2,   // [0-9]+.[0-9]+
        T_STRING = 3,  // "..."
        T_CHAR = 4,    // '...'
        T_QUOTE = 5,   // :
        T_IDENT = 6,
        T_LPAREN = 7,  // (
        T_RPAREN = 8,  // )
        T_LBRACK = 9,  // [
        T_RBRACK = 10; // ]

    struct Token {
        ustring name;
        u32 id, line, column;

        operator bool() const;
    };

    struct TokenView {
        vector<Token>& _cache;
        u32 index;
    public:
        TokenView(vector<Token>& cache);
        const Token& peek() const;
        const Token& read();
        void rewind();
    };

    Token lex(Source::View& view);
}

void write(stream& io, const basil::Token& token);

#endif