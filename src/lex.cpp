#include "lex.h"
#include "source.h"
#include "vec.h"
#include "errors.h"

namespace basil {
    static const Token NONE{ "", T_NONE, 0, 0 };

    Token::operator bool() const {
        return id != T_NONE;
    }
    
    TokenView::TokenView(vector<Token>& cache):
        _cache(cache), index(0) {
        //
    }

    const Token& TokenView::peek() const {
        if (index >= _cache.size()) return NONE;
        else return _cache[index];
    }

    const Token& TokenView::read() {
        if (index >= _cache.size()) return NONE;
        else return _cache[index ++];
    }

    void TokenView::rewind() {
        if (index > 0) index --;
    }

    static bool isdelim(uchar c) {
        return isspace(c) || c == '(' || c == ')' || c == '[' || c == ']';
    }

    Token lex(Source::View& view) {
        Token result = NONE;
        if (view.peek() == '\0') {
            return NONE;
        }
        else if (view.peek() == '#') { // consume comment
            view.read();
            while (view.peek() != '\n') view.read();
        }
        else if (isspace(view.peek())) {
            while (isspace(view.peek())) view.read();
        }
        else if (view.peek() == ':') {
            result.id = T_QUOTE;
            result.line = view.line(), result.column = view.column();
            result.name = ":";
            view.read();
        }
        else if (view.peek() == '(') {
            result.id = T_LPAREN;
            result.line = view.line(), result.column = view.column();
            result.name = "(";
            view.read();
        }
        else if (view.peek() == ')') {
            result.id = T_RPAREN;
            result.line = view.line(), result.column = view.column();
            result.name = ")";
            view.read();
        }
        else if (view.peek() == '[') {
            result.id = T_LBRACK;
            result.line = view.line(), result.column = view.column();
            result.name = "[";
            view.read();
        }
        else if (view.peek() == ']') {
            result.id = T_RBRACK;
            result.line = view.line(), result.column = view.column();
            result.name = "]";
            view.read();
        }
        else if (view.peek() == '"') {
            result.id = T_STRING;
            result.line = view.line(), result.column = view.column();
            view.read();
            while (view.peek() != '"') {
                if (view.peek() == '\n') {
                    err(PHASE_LEX, view.line(), view.column(),
                        "Line breaks are not permitted within string constants.");
                    view.read();
                    return NONE;
                }
                else if (view.peek() == '\0') {
                    err(PHASE_LEX, view.line(), view.column(),
                        "Unexpected end of file within string constant.");
                    return NONE;
                }
                else if (view.peek() == '\\') {
                    view.read();
                    switch (view.peek()[0]) {
                        case '"': result.name += '"'; break;
                        case '\'': result.name += '\''; break;
                        case '\\': result.name += '\\'; break;
                        case 'n': result.name += '\n'; break;
                        case 't': result.name += '\t'; break;
                        case 'r': result.name += '\r'; break;
                        case '0': result.name += '\0'; break;
                        default: 
                            err(PHASE_LEX, view.line(), view.column(),
                                "Unknown escape sequence '\\", view.peek(), "'.");
                            view.read();
                            return NONE;
                    }
                    view.read();
                }
                else result.name += view.read();
            }
            view.read();
        }
        else if (view.peek() == '\'') {
            result.id = T_CHAR;
            result.line = view.line(), result.column = view.column();
            view.read();
            if (view.peek() == '\n') {
                err(PHASE_LEX, view.line(), view.column(),
                    "Line breaks are not permitted within character constants.");
                view.read();
                return NONE;
            }
            else if (view.peek() == '\0') {
                err(PHASE_LEX, view.line(), view.column(),
                    "Unexpected end of file within character constant.");
                return NONE;
            }
            else if (view.peek() == '\\') {
                view.read();
                switch (view.peek()[0]) {
                    case '"': result.name += '"'; break;
                    case '\'': result.name += '\''; break;
                    case '\\': result.name += '\\'; break;
                    case 'n': result.name += '\n'; break;
                    case 't': result.name += '\t'; break;
                    case 'r': result.name += '\r'; break;
                    case '0': result.name += '\0'; break;
                    default: 
                        err(PHASE_LEX, view.line(), view.column(),
                            "Unknown escape sequence '\\", view.peek(), "'.");
                        view.read();
                        return NONE;
                }
                view.read();
            }
            else result.name += view.read();
            
            if (view.peek() != '\'') {
                err(PHASE_LEX, view.line(), view.column(),
                    "More than one character in character constant.");
                view.read();
                return NONE;
            }
            else view.read();
        }
        else if (isdigit(view.peek())) {
            result.id = T_INT;
            result.line = view.line(), result.column = view.column();
            result.name += view.read();
            while (isdigit(view.peek())) {
                result.name += view.read();
            }
            if (view.peek() == '.') {
                result.id = T_FLOAT;
                result.name += view.read();
                while (isdigit(view.peek())) {
                    result.name += view.read();
                }
            }
            if (!isdelim(view.peek())) {
                err(PHASE_LEX, view.line(), view.column(),
                    "Unexpected character '", view.peek(), "' in numeric literal.");
                view.read();
                return NONE;
            }
        }
        else if (isprint(view.peek())) {
            if (view.peek() == '_') {
                err(PHASE_LEX, view.line(), view.column(),
                    "Identifiers cannot start with '_'.");
                view.read();
                return NONE;
            }
            result.id = T_IDENT;
            result.line = view.line(), result.column = view.column();
            result.name += view.read();
            while (!isdelim(view.peek())) {
                result.name += view.read();
            }
        }
        else {
            err(PHASE_LEX, view.line(), view.column(),
                "Unexpected character '", view.peek(), "' in input.");
            view.read();
            return NONE;
        }
        return result;
    }
}

void write(stream& io, const basil::Token& token) {
    write(io, "[", token.id, ": ", token.name, "]");
}