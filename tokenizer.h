#include <cctype>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

struct Source {
    enum SourceType { NONE, STACK, ARRAY, CONSTANT } type;
    int64_t specifier;
};

bool operator==(Source, Source::SourceType);

enum PayloadType { NONE, NORMAL, SPECIAL };
enum SpecialPayload {
    INT,
    CHAR,
    STACK
};

struct Instruction {
    std::string raw;
    
    union {
        int64_t normal;
        SpecialPayload special;
    } payload;
    
    PayloadType payload_type;
    
    Source source;
    
    size_t line;
    size_t column;
};

std::ostream& operator<<(std::ostream&, const Source::SourceType&);
std::ostream& operator<<(std::ostream&, const Instruction&);

class Tokenizer {
    std::map<char, std::pair<size_t, Source>> VALID_OPS = {
        { '$',  { 1, { Source::ARRAY,    0 } } },
        { '~',  { 1, { Source::NONE,     0 } } },
        { '"',  { 0, { Source::NONE,     0 } } },
        { '<',  { 0, { Source::CONSTANT, 1 } } },
        { '>',  { 0, { Source::CONSTANT, 1 } } },
        { '@',  { 0, { Source::ARRAY,    0 } } },
        { '&',  { 0, { Source::STACK,    0 } } },
        { '#',  { 0, { Source::ARRAY,    0 } } },
        { '(',  { 0, { Source::CONSTANT, 0 } } },
        { ')',  { 0, { Source::NONE,     0 } } },
        { '{',  { 0, { Source::CONSTANT, 0 } } },
        { '}',  { 0, { Source::NONE,     0 } } },
        { '[',  { 0, { Source::CONSTANT, 0 } } },
        { ']',  { 0, { Source::NONE,     0 } } },
        { '?',  { 0, { Source::ARRAY,    0 } } },
        
        // arithmetic operators
        { '+',  { 0, { Source::STACK,    0 } } },
        { '-',  { 0, { Source::STACK,    0 } } },
        { ';',  { 0, { Source::STACK,    0 } } },
        { '/',  { 0, { Source::STACK,    0 } } },
        { '\\', { 0, { Source::STACK,    0 } } },
        { '*',  { 0, { Source::STACK,    0 } } },
        { '%',  { 0, { Source::STACK,    0 } } },
        { '^',  { 0, { Source::STACK,    0 } } },
        { '=',  { 0, { Source::STACK,    0 } } },
    };
    
    std::map<char, SpecialPayload> SPECIAL_PAYLOADS = {
        { '.', SpecialPayload::INT },
        { ':', SpecialPayload::CHAR },
        { '!', SpecialPayload::STACK },
    };
    
    std::string code;
    std::vector<Instruction> tokens;
    size_t pointer = 0;
    
    inline char current() {
        return code[pointer];
    }
    
    inline char next() {
        return code[++pointer];
    }
    
    bool running() {
        return pointer < code.size();
    }
    
    bool has_number() {
        return isdigit(current());
    }
    
    bool starts_number() {
        return has_number() || current() == '_';
    }
    
    int64_t read_number();
    
    void step();
    
public:

    Tokenizer(std::string);
    
    std::vector<Instruction> tokenize();
    void debug();
};