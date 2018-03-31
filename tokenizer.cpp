#include "tokenizer.h"


bool operator==(Source a, Source::SourceType b) {
    return a.type == b;
}

std::ostream& operator<<(std::ostream& stream, const Source::SourceType& type) {
    switch(type) {
        case Source::NONE:
            stream << "NONE";
            break;
        case Source::STACK:
            stream << "STACK";
            break;
        case Source::ARRAY:
            stream << "ARRAY";
            break;
        case Source::CONSTANT:
            stream << "CONSTANT";
            break;
        default:
            stream << "(invalid/undefined)";
    }
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Instruction& inst) {
    stream << "Instruction(`" << inst.raw << "`, ";
    switch(inst.payload_type) {
        case PayloadType::NONE:
            stream << "{}";
            break;
        
        case PayloadType::NORMAL:
            stream << inst.payload.normal;
            break;
        
        case PayloadType::SPECIAL:
            stream << inst.payload.special;
            break;
    }
    stream << ")";
    return stream;
}


Tokenizer::Tokenizer(std::string code) : code(code) {}

int64_t Tokenizer::read_number() {
    int64_t build = 0;
    int sign = 1;
    
    if(current() == '_') {
        sign = -1;
        next();
    }
    
    while(has_number()) {
        build *= 10;
        build += current() - '0';
        next();
    }
    
    return build * sign;
}

void Tokenizer::step() {
    auto it = VALID_OPS.find(current());
    
    if(it == VALID_OPS.end()) {
        next();
        return;
    }
    
    Instruction res;    
    std::string build = "";
    res.source = it->second.second;
    res.payload.normal = 0;
    res.payload_type = PayloadType::NONE;
    
    if(current() == '"') {
        build += current();
        build += next();
        
        while(current() != '"') {
            build += next();
        }
        
        next();
    }
    else {
        int64_t consume = it->second.first;
        
        while(consume >= 0) {
            build += current();
            next();
            consume--;
        }
        
        if(starts_number()) {
            res.payload_type = PayloadType::NORMAL;
            res.payload.normal = read_number();
        }
        else {
            auto spec = SPECIAL_PAYLOADS.find(current());
            
            if(spec != SPECIAL_PAYLOADS.end()) {
                res.payload.special = spec->second;
                res.payload_type = PayloadType::SPECIAL;
            }
        }
    }
    
    res.raw = build;
    
    // todo: implement
    res.line = res.column = 42;
        
    tokens.push_back(res);
}

std::vector<Instruction> Tokenizer::tokenize() {
    while(running()) {
        step();
    }
    
    return tokens;
}

void Tokenizer::debug() {
    for(Instruction i : tokens) {
        std::cout << i << std::endl;
    }
}