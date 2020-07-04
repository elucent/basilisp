#include "source.h"

namespace basil {
    void Source::add(uchar c) {
        if (c == '\t') text += "    "; // expand tabs to 4 spaces
        else text += c;
        if (c == '\n') lines.push(text.size());
    }

    Source::Source() {
        lines.push(0);
    }

    Source::Source(const char* path) {
        lines.push(0);
        file f(path, "r");
        load(f);
    }

    Source::Source(stream& f) {
        lines.push(0);
        load(f);
    }

    Source::View::View(const Source* src_in): src(src_in), 
        _line(0), _column(0), _data(&src->text[0]) {
        //  
    }

    Source::View::View(const Source* src_in, u32 line, u32 column): 
        src(src_in), _line(line), _column(column), 
        _data(&src->text[src->lines[line] + column]) {
        //  
    }

    void Source::View::rewind() {
        if (_column == 0 && _line > 0) {
            -- _data;
            -- _line, _column = _data - (&src->text[src->lines[_line]]);
        }
        else if (_column > 0) -- _data, -- _column;
    }

    const_slice<uchar> Source::View::operator[](pair<u32, u32> range) const {
        return { range.second - range.first, _data + range.first };
    }

    uchar Source::View::read() {
        uchar c = peek();
        ++ _column;
        if (_column >= src->line(_line).size() && _line < src->lines.size() - 1) {
            _column = 0, ++ _line;
        } 
        return c;
    }

    uchar Source::View::peek() const {
        if (_line >= src->lines.size()) return '\0';
        if (_line == src->lines.size() - 1 
            && _column >= src->line(_line).size()) return '\0';
        return src->line(_line)[_column];
    }

    u32 Source::View::line() const {
        return _line + 1;
    }

    u32 Source::View::column() const {
        return _column + 1;
    }

    const Source* Source::View::source() const {
        return src;
    }

    void Source::load(stream& f) {
        uchar c;
        while (f.peek()) read(f, c), add(c);
    }

    void Source::add(const ustring& line) {
        for (u32 i = 0; i < line.size(); ++ i) add(line[i]);
    }

    slice<uchar> Source::line(u32 line) {
        u32 start = lines[line], end = line + 1 >= lines.size() ?   
            text.size() : lines[line + 1];
        return { end - start, &text[start] };
    }

    const_slice<uchar> Source::line(u32 line) const {
        u32 start = lines[line], end = line + 1 >= lines.size() ?   
            text.size() : lines[line + 1];
        return { end - start, &text[start] };
    }

    u32 Source::size() const {
        return text.size();
    }

    Source::View Source::view() const {
        return Source::View(this);
    }

    Source::View Source::expand(stream& io) {
        Source::View view(this, lines.size() - 1, 0);
        while (io.peek() && io.peek() != '\n') {
            uchar c;
            read(io, c);
            add(c);
        }
        if (io.peek() == '\n') add(io.read());
        return view;
    }
}
    
void write(stream& io, const basil::Source& src) {
    for (u32 i = 0; i < src.size(); ++ i) write(io, src.line(i));
}

void read(stream& io, basil::Source& src) {
    src.load(io);
}