#include "errors.h"
#include "source.h"

namespace basil {
    Error::Error(): src(nullptr), line(0), column(0) {
        //
    }

    void Error::format(stream& io) const {
        writeln(io, message);
        if (src) {
            write(io, "    ", src->line(line - 1));
            write(io, "    ");
            for (u32 i = 0; i < column - 1; i ++) write(io, " ");
            writeln(io, "^");
        }
    }   

    static vector<Error> errors;
    static set<ustring> messages;

    static vector<vector<Error>> errorFrames;
    static vector<set<ustring>> frameMessages;

    void catchErrors() {
        errorFrames.push({});
        frameMessages.push({});
    }

    void releaseErrors() {
        vector<Error> removed = errorFrames.back();
        errorFrames.pop();
        frameMessages.pop();
        for (const Error& e : removed) reportError(e);
    }

    void discardErrors() {
        // if (countErrors()) printErrors(_stdout);
        errorFrames.pop();
        frameMessages.pop();
    }

    void prefixPhase(buffer& b, Phase phase) {
        switch (phase) {
            case PHASE_LEX:
                write(b, "[TOKEN ERROR]");
                break;
            case PHASE_PARSE:
                write(b, "[PARSE ERROR]");
                break;
            case PHASE_TYPE:
                write(b, "[TYPE ERROR]");
                break;
            default:
                break;
        }
    }

    static Source* _src;

    void useSource(Source* src) {
        _src = src;
    }

    Source* currentSource() {
        return _src;
    }

    void reportError(const Error& error) {
        ustring s;
        buffer b = error.message;
        read(b, s);
        vector<Error>& es = errorFrames.size() 
            ? errorFrames.back() : errors;
        set<ustring>& ms = frameMessages.size() 
            ? frameMessages.back() : messages;
        if (ms.find(s) == ms.end()) {
            ms.insert(s);
            es.push(error);
            if (!es.back().src && _src) es.back().src = _src;
        }
    }

    u32 countErrors() {
        vector<Error>& es = errorFrames.size() 
            ? errorFrames.back() : errors;
        return es.size();
    }

    void printErrors(stream& io) {
        writeln(io, countErrors(), " error", countErrors() != 1 ? "s" : "");
        for (const Error& e : (errorFrames.size() 
            ? errorFrames.back() : errors)) write(io, e);
    }

    Error& lastError() {
        return (errorFrames.size() ? errorFrames.back().back() : errors.back());
    }
}

void write(stream& io, const basil::Error& e) {
    e.format(io);
}