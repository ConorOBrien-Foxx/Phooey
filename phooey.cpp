#include "tokenizer.h"
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <list>
#include <thread>
#include <chrono>
#include <streambuf>
#include <string>

// modified from http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
std::string read_file(const char *filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
        if(in) {
            return std::string(
                std::istreambuf_iterator<char>(in),
                std::istreambuf_iterator<char>()
            );
        }
    throw errno;
}

void sleep(int ms) {
    std::chrono::milliseconds duration(ms);
    
    std::this_thread::sleep_for(duration);
}

int64_t random_between(int64_t lower, int64_t upper) {
    return rand() % (upper - lower + 1) + lower;
}

int64_t time_since_epoch() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

const size_t TAPE_SIZE = 30000;

class Tape {
    int64_t data[TAPE_SIZE];
    
public:
    size_t pointer = 0;
    size_t furthest = 0;
    
    int64_t &operator[](size_t index) {
        return data[index];
    }
    
    void left(int64_t amount) {
        right(-amount);
    }
    
    // todo: wrapping
    void right(int64_t amount) {
        pointer += amount;
        
        if(pointer > furthest)
            furthest = pointer;
    }
    
    int64_t &current() {
        return data[pointer];
    }
};

std::ostream& operator<<(std::ostream& stream, Tape& tape) {
    stream << "[";
    for(size_t i = 0; i <= tape.furthest; i++) {
        if(i == tape.pointer) {
            stream << ">";
        }
        stream << tape[i];
        if(i == tape.pointer) {
            stream << "<";
        }
        if(i != tape.furthest) {
            stream << " ";
        }
    }
    return stream << "]";
}

struct CallStackInfo {
    size_t start;
    int64_t compare;
};

class Phooey {
    Tape tape;
    std::list<int64_t> stack = {};
    std::list<CallStackInfo> call_stack = {};
    std::map<size_t, size_t> if_jumps = {};
    std::map<size_t, size_t> loop_jumps = {};
    std::string code;
    std::vector<Instruction> ops;
    size_t pointer = 0;
    
    void step();
    
    Instruction& operator[](size_t ind) {
        return ops[ind];
    }
    
    inline Instruction& current() {
        return ops[pointer];
    }
    
    inline bool running() {
        return pointer < ops.size();
    }
    
    inline void add_call(size_t source, int64_t compare) {
        call_stack.push_back(CallStackInfo { source, compare });
    }
    
    int64_t get_operand(Instruction&);
    int64_t pop();
    void push(int64_t);
    
    void advance() {
        pointer++;
    }
    
public:
    
    Phooey(std::string);
    void run();
    void debug();
};

Phooey::Phooey(std::string code) : code(code) {
    Tokenizer temp(code);
    
    ops = temp.tokenize();
    
    // std::cout << ">>> TOKEN DUMP <<<" << std::endl;
    // temp.debug();
    // std::cout << ">>> END TOKEN DUMP <<<" << std::endl << std::endl;
    
    std::list<size_t> if_stack = {};
    std::list<size_t> jump_stack = {};
    for(size_t i = 0; i < ops.size(); i++) {
        Instruction op = ops[i];
        
        if(op.raw == "{") {
            if_stack.push_back(i);
        }
        else if(op.raw == "}") {
            size_t start = if_stack.back();
            
            // if_jumps[i] = start;
            if_jumps[start] = i;
            
            if_stack.pop_back();
        }
        
        else if(op.raw == "[") {
            jump_stack.push_back(i);
        }
        else if(op.raw == "]") {
            size_t start = jump_stack.back();
            
            loop_jumps[start] = i;
            loop_jumps[i] = start - 1;
            
            jump_stack.pop_back();
        }
    }
    
    // read if locations
    // temp.debug();
}

int64_t Phooey::pop() {
    if(stack.empty()) {
        return 0;
    }
    else {
        int64_t res = stack.back();
        stack.pop_back();
        return res;
    }
}

void Phooey::push(int64_t el) {
    stack.push_back(el);
}

int64_t read_int(std::istream& stream) {
    int64_t res;
    stream >> res;
    return res;
}

int64_t read_int() {
    return read_int(std::cin);
}

char read_char(std::istream& stream) {
    return stream.get();
}

char read_char() {
    return read_char(std::cin);
}

int64_t Phooey::get_operand(Instruction &op) {
    if(op.payload_type == PayloadType::NORMAL) {
        return op.payload.normal;
    }
    else if(op.payload_type == PayloadType::SPECIAL) {
        switch(op.payload.special) {
            case SpecialPayload::INT:
                return read_int();
            
            case SpecialPayload::CHAR:
                return read_char();
            
            case SpecialPayload::STACK:
                op.payload_type = PayloadType::NORMAL;
                op.payload.normal = pop();
                return op.payload.normal;
            
            default:
                std::cerr << "Unhandled SpecialPayload" << std::endl;
                return 42;
        }
    }
    else if(op.source == Source::STACK) {
        return pop();
    }
    else if(op.source == Source::ARRAY) {
        return tape.current();
    }
    else if(op.source == Source::NONE) {
        return 0;
    }
    else if(op.source == Source::CONSTANT) {
        return op.source.specifier;
    }
    else {
        std::cerr << "Unhandled operator:" << std::endl;
        std::cerr << op << std::endl;
        std::cerr << "op.source = " << op.source.type << std::endl;
        return 1;
    }
}

void Phooey::step() {
    Instruction &cur = current();
    char id = cur.raw[0];
    int64_t op = get_operand(cur);
    
    // handle the operator
    if(id == '"') {
        std::cout << cur.raw.substr(1, cur.raw.size() - 2);
    }
    else if(id == '&') {
        tape.current() = op;
    }
    else if(id == '*') {
        tape.current() *= op;
    }
    else if(id == '+') {
        tape.current() += op;
    }
    else if(id == '-') {
        tape.current() -= op;
    }
    else if(id == ';') {
        tape.current() = op - tape.current();
    }
    else if(id == '=') {
        tape.current() = tape.current() == op;
    }
    else if(id == '/') {
        tape.current() /= op;
    }
    else if(id == '\\') {
        tape.current() = op / tape.current();
    }
    else if(id == '%') {
        tape.current() %= op;
    }
    else if(id == '^') {
        tape.current() = pow(tape.current(), op);
    }
    else if(id == '>') {
        tape.right(op);
    }
    else if(id == '<') {
        tape.left(op);
    }
    else if(id == '@') {
        push(op);
    }
    else if(id == '#') {
        sleep(op * 1000);
    }
    else if(id == '(') {
        add_call(pointer, op);
    }
    else if(id == ')') {
        CallStackInfo info = call_stack.back();
        if(tape.current() != info.compare) {
            pointer = info.start;
        }
        else {
            call_stack.pop_back();
        }
    }
    else if(id == '?') {
        debug();
    }
    else if(id == '{') {
        if(tape.current() != op) {
            pointer = if_jumps[pointer];
        }
    }
    else if(id == '}') {
        // no-op
    }
    else if(id == '[') {
        if(tape.current() == op) {
            pointer = loop_jumps[pointer];
        }
    }
    else if(id == ']') {
        if(tape.current() != op) {
            pointer = loop_jumps[pointer];
        }
    }
    else if(id == '$') {
        char mode = cur.raw[1];
        
        if(mode == 'h') {
            std::cout << std::hex << op << std::dec;
        }
        else if(mode == 'i') {
            std::cout << op;
        }
        else if(mode == 'c') {
            std::cout << (char) op;
        }
        else {
            std::cerr << "Unknown mode for '$'" << std::endl;
        }
    }
    else if(id == '~') {
        char original = cur.raw[1];
        char mode = tolower(original);
        
        int64_t res = 0;
        bool modify = true;
        
        // is there input left?
        if(mode == 'i') {
            res = !std::cin.eof();
        }
        // random integer
        else if(mode == '?') {
            res = random_between(0, op);
        }
        // random integer 2
        else if(mode == 'r') {
            res = random_between(tape.current(), op);
        }
        else if(mode == 's') {
            sleep(op);
            modify = false;
        }
        // current time, in milliseconds
        else if(mode == 't') {
            res = time_since_epoch();
        }
        
        if(modify) {
            // lowercase -> update tape
            if(mode == original) {
                tape.current() = res;
            }
            // uppercase -> push to stack
            else {
                push(res);
            }
        }
    }
    else {
        std::cerr << "Unimplemented: " << id << std::endl;
        exit(-1);
    }
    
    advance();
}

void Phooey::debug() {
    std::cout << ">>> DEBUG <<<" << std::endl;
    std::cout << "Tape: " << tape << std::endl;
    std::cout << "Stack:" << std::endl;
    for(int64_t i : stack) {
        std::cout << i << std::endl;
    }
}

void Phooey::run() {
    while(running()) {
        step();
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    
    srand(time_since_epoch());
    
    std::string code = read_file(argv[1]);
    
    Phooey instance(code);
    
    instance.run();
    
    // instance.debug();
}