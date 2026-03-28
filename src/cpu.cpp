#include "cpu.h"
#include "peripherals.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <iomanip>

void to_upper_str(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::toupper(c); });
}

std::string trim_str(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) return "";
    size_t last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, (last - first + 1));
}

CPU16::CPU16(std::shared_ptr<Peripherals> peri)
    : mem(MEM_SIZE, 0), reg(REG_COUNT, 0), pc(0), running(true), flags(0), peripherals(peri) {
    reg[SP] = 0xFFFE;
    reg[BP] = 0xFFFE;
    set_flag(IF_MASK, true);

    opcodes = {
        {"HLT", Opcode::HLT}, {"NOP", Opcode::NOP}, {"MOV", Opcode::MOV}, {"XCHG", Opcode::XCHG}, {"LEA", Opcode::LEA},
        {"ADD", Opcode::ADD}, {"SUB", Opcode::SUB}, {"MUL", Opcode::MUL}, {"DIV", Opcode::DIV},{"MOD", Opcode::MOD},
        {"AND", Opcode::AND}, {"OR", Opcode::OR}, {"XOR", Opcode::XOR}, {"SHL", Opcode::SHL},
        {"SHR", Opcode::SHR},  {"CBW", Opcode::CBW}, {"NOT", Opcode::NOT}, {"INC", Opcode::INC}, {"DEC", Opcode::DEC},
        {"CMP", Opcode::CMP}, {"JMP", Opcode::JMP}, {"JMPE", Opcode::JMPE}, {"JMPNE", Opcode::JMPNE},
        {"JMPG", Opcode::JMPG}, {"JMPL", Opcode::JMPL}, {"JMPGE", Opcode::JMPGE}, {"JMPLE", Opcode::JMPLE},
        {"PUSH", Opcode::PUSH}, {"POP", Opcode::POP}, {"PUSHF", Opcode::PUSHF}, {"POPF", Opcode::POPF},
        {"PUSHA", Opcode::PUSHA}, {"POPA", Opcode::POPA}, {"ENTER", Opcode::ENTER}, {"LEAVE", Opcode::LEAVE},
        {"NEG", Opcode::NEG}, {"TEST", Opcode::TEST}, {"CALL", Opcode::CALL}, {"RET", Opcode::RET},
        {"INT", Opcode::INT}, {"IRET", Opcode::IRET}, {"IN", Opcode::IN}, {"OUT", Opcode::OUT},
        {"STI", Opcode::STI}, {"CLI", Opcode::CLI}
    };
}

void CPU16::set_pc(size_t address) {
    pc = address;
}

bool CPU16::is_running() const {
    return running;
}

void CPU16::set_flag(uint16_t mask, bool value) {
    if (value) flags |= mask;
    else flags &= ~mask;
}

bool CPU16::get_flag(uint16_t mask) const {
    return (flags & mask) != 0;
}

Operand CPU16::parse_operand_string(const std::string& op_str_raw, const std::unordered_map<std::string, size_t>& labels) {
    std::string s = trim_str(op_str_raw);
    to_upper_str(s);

    if (s.length() > 2 && s.front() == '[' && s.back() == ']') {
        std::string inner = s.substr(1, s.length() - 2);
        inner.erase(std::remove_if(inner.begin(), inner.end(), ::isspace), inner.end());

        size_t plus_pos = inner.find('+');
        size_t minus_pos = inner.find('-');
        size_t op_pos = (plus_pos != std::string::npos) ? plus_pos : minus_pos;

        if (op_pos != std::string::npos && op_pos > 0) {
            std::string reg_str = inner.substr(0, op_pos);
            std::string offset_str = inner.substr(op_pos);
            int base_reg_num = -1;
            if (reg_str == "BP") base_reg_num = BP;
            else if (reg_str == "SP") base_reg_num = SP;
            else if (reg_str.rfind("R", 0) == 0) base_reg_num = std::stoi(reg_str.substr(1));
            else throw std::runtime_error("Invalid register: " + reg_str);

            int offset = std::stoi(offset_str, nullptr, 0);
            int32_t packed = (static_cast<int16_t>(offset) << 16) | base_reg_num;
            return { OperandType::MEM_REG_OFFSET, packed, "" };
        }
        else {
            int reg_num = -1;
            if (inner == "BP") reg_num = BP;
            else if (inner == "SP") reg_num = SP;
            else if (inner.rfind("R", 0) == 0) reg_num = std::stoi(inner.substr(1));
            return { OperandType::MEM_INDIRECT, reg_num, "" };
        }
    }
    else if (s == "BP") {
        return { OperandType::REG, BP, "" };
    }
    else if (s == "SP") {
        return { OperandType::REG, SP, "" };
    }
    else if (s.rfind("R", 0) == 0) {
        return { OperandType::REG, std::stoi(s.substr(1)), "" };
    }
    else if (s.length() == 3 && s.front() == '\'' && s.back() == '\'') {
        return { OperandType::IMM, static_cast<int>(s[1]), "" };
    }
    else {
        try {
            size_t pos;
            int val = std::stoi(s, &pos, 0);
            if (pos == s.size()) return { OperandType::IMM, val, "" };
        }
        catch (...) {}
        std::string upper_s = s;
        to_upper_str(upper_s);
        if (labels.count(upper_s)) return { OperandType::IMM, static_cast<int>(labels.at(upper_s)), "" };
        return { OperandType::LABEL, 0, s };
    }
}

uint16_t CPU16::get_value(const Operand& operand) {
    switch (operand.type) {
    case OperandType::REG:
        return reg.at(operand.value) & 0xFFFF;
    case OperandType::IMM:
        return operand.value & 0xFFFF;
    case OperandType::MEM_INDIRECT: {
        uint16_t addr = reg.at(operand.value) & 0xFFFF;

        return mem.at(addr);
    }
    case OperandType::MEM_REG_OFFSET: {
        uint16_t base_reg = operand.value & 0xFFFF;
        int16_t offset = operand.value >> 16;

        uint16_t addr = (reg.at(base_reg) + offset) & 0xFFFF;

        return mem.at(addr);
    }
    default: throw std::runtime_error("Invalid get_value");
    }
}

void CPU16::set_value(const Operand& operand, uint16_t value) {
    switch (operand.type) {
    case OperandType::REG:
        reg.at(operand.value) = value;
        break;
    case OperandType::MEM_INDIRECT: {
        uint16_t addr = reg.at(operand.value) & 0xFFFF;

        mem.at(addr) = value;
        break;
    }
    case OperandType::MEM_REG_OFFSET: {
        uint16_t base_reg = operand.value & 0xFFFF;
        int16_t offset = operand.value >> 16;

        uint16_t addr = (reg.at(base_reg) + offset) & 0xFFFF;

        mem.at(addr) = value;
        break;
    }
    default: throw std::runtime_error("Invalid set_value");
    }
}

void CPU16::trigger_interrupt(uint16_t vector_num) {
    if (!get_flag(IF_MASK)) return;

    reg[SP]--;
    mem.at(reg[SP]) = flags;

    reg[SP]--;
    mem.at(reg[SP]) = static_cast<uint16_t>(pc);

    set_flag(IF_MASK, false);

    pc = mem.at(IVT_ADDRESS + vector_num);
}

void CPU16::load_program(const std::string& filename, size_t offset) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("File error");
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        size_t cp = line.find(';');
        if (cp != std::string::npos) line = line.substr(0, cp);
        line = trim_str(line);
        if (!line.empty()) lines.push_back(line);
    }
    std::unordered_map<std::string, size_t> labels;
    size_t ptr = offset;
    for (auto& l : lines) {
        size_t cl = l.find(':');
        if (cl != std::string::npos) {
            std::string n = trim_str(l.substr(0, cl));
            to_upper_str(n);
            labels[n] = ptr;
            l = trim_str(l.substr(cl + 1));
        }
        if (l.empty()) continue;
        std::string op = l.substr(0, l.find_first_of(" \t"));
        to_upper_str(op);
        if (op == "DB") {
            std::string as = trim_str(l.substr(op.length()));
            bool in_s = false;
            as += ',';
            std::string seg;
            for (char c : as) {
                if (c == '"' || c == '\'') in_s = !in_s;
                if (c == ',' && !in_s) {
                    seg = trim_str(seg);
                    if (!seg.empty()) {
                        if (seg.front() == '"' || seg.front() == '\'') ptr += (seg.length() - 2);
                        else ptr++;
                    }
                    seg = "";
                }
                else seg += c;
            }
        }
        else if (op == "DW") {
            std::stringstream ss(trim_str(l.substr(op.length())));
            std::string t;
            while (std::getline(ss, t, ',')) ptr++;
        }
        else {
            ptr++;
            size_t fs = l.find_first_of(" \t");
            if (fs != std::string::npos) {
                std::stringstream ss(trim_str(l.substr(fs + 1)));
                std::string a;
                while (std::getline(ss, a, ',')) ptr += 3;
            }
        }
    }
    ptr = offset;
    for (const auto& l_orig : lines) {
        std::string l = l_orig;
        size_t cl = l.find(':');
        if (cl != std::string::npos) l = trim_str(l.substr(cl + 1));
        if (l.empty()) continue;
        size_t fs = l.find_first_of(" \t");
        std::string op_s = l.substr(0, fs);
        std::string args = (fs != std::string::npos) ? trim_str(l.substr(fs + 1)) : "";
        to_upper_str(op_s);
        if (op_s == "DB") {
            bool in_s = false;
            std::string seg;
            args += ',';
            for (char c : args) {
                if (c == '"') in_s = !in_s;
                if (c == ',' && !in_s) {
                    seg = trim_str(seg);
                    if (!seg.empty()) {
                        if (seg.front() == '"' || seg.front() == '\'') {
                            for (size_t i = 1; i < seg.length() - 1; ++i) mem[ptr++] = static_cast<uint8_t>(seg[i]);
                        }
                        else mem[ptr++] = static_cast<uint8_t>(std::stoi(seg, nullptr, 0));
                    }
                    seg = "";
                }
                else seg += c;
            }
        }
        else if (op_s == "DW") {
            std::stringstream ss(args);
            std::string t;
            while (std::getline(ss, t, ',')) mem[ptr++] = static_cast<uint16_t>(std::stoi(trim_str(t), nullptr, 0));
        }
        else {
            mem[ptr++] = static_cast<uint16_t>(opcodes.count(op_s) ? opcodes.at(op_s) : Opcode::INVALID);
            if (!args.empty()) {
                std::stringstream ss(args);
                std::string a;
                while (std::getline(ss, a, ',')) {
                    Operand o = parse_operand_string(a, labels);
                    if (o.type == OperandType::LABEL) {
                        std::string un = o.name; to_upper_str(un);
                        if (labels.count(un)) { o.value = labels.at(un); o.type = OperandType::IMM; }
                        else throw std::runtime_error("Label error");
                    }
                    mem[ptr++] = static_cast<uint16_t>(o.type);
                    uint32_t v32 = static_cast<uint32_t>(o.value);
                    mem[ptr++] = v32 & 0xFFFF;
                    mem[ptr++] = (v32 >> 16) & 0xFFFF;
                }
            }
        }
    }
}

Operand CPU16::fetch_operand() {
    Operand op;
    op.type = static_cast<OperandType>(mem[pc++]);
    uint16_t low = mem[pc++];
    uint16_t high = mem[pc++];
    op.value = (static_cast<uint32_t>(high) << 16) | low;
    return op;
}

void CPU16::op_hlt() {
    running = false;
}

void CPU16::op_nop() {
}

void CPU16::op_mov() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();
    set_value(dst, get_value(src));
}

void CPU16::op_xchg() {
    Operand o1 = fetch_operand();
    Operand o2 = fetch_operand();
    uint16_t v1 = get_value(o1);
    uint16_t v2 = get_value(o2);
    set_value(o1, v2);
    set_value(o2, v1);
}

void CPU16::op_lea() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();

    uint16_t effective_address = 0;

    switch (src.type) {
    case OperandType::MEM_INDIRECT:
        effective_address = reg.at(src.value) & 0xFFFF;
        break;

    case OperandType::MEM_REG_OFFSET: {
        uint16_t base_reg = src.value & 0xFFFF;
        int16_t offset = src.value >> 16;
        effective_address = (reg.at(base_reg) + offset) & 0xFFFF;
        break;
    }

    case OperandType::IMM:
    case OperandType::LABEL:
        effective_address = src.value & 0xFFFF;
        break;

    default:
        throw std::runtime_error("Invalid source operand type for LEA instruction");
    }

    set_value(dst, effective_address);
}

void CPU16::op_add() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();
    set_value(dst, get_value(dst) + get_value(src));
}

void CPU16::op_sub() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();
    set_value(dst, get_value(dst) - get_value(src));
}

void CPU16::op_mul() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();
    set_value(dst, get_value(dst) * get_value(src));
}

void CPU16::op_div() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();
    uint16_t v = get_value(src);
    if (v == 0) { 
        trigger_interrupt(0); 
        return; 
    }
    set_value(dst, get_value(dst) / v);
}

void CPU16::op_mod() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();
    uint16_t v = get_value(src);
    if (v == 0) { 
        trigger_interrupt(0); 
        return; 
    }
    set_value(dst, get_value(dst) % v);
}

void CPU16::op_and() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();
    set_value(dst, get_value(dst) & get_value(src));
}

void CPU16::op_or() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();
    set_value(dst, get_value(dst) | get_value(src));
}

void CPU16::op_xor() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();
    set_value(dst, get_value(dst) ^ get_value(src));
}

void CPU16::op_shl() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();
    set_value(dst, get_value(dst) << get_value(src));
}

void CPU16::op_shr() {
    Operand dst = fetch_operand();
    Operand src = fetch_operand();
    set_value(dst, get_value(dst) >> get_value(src));
}

void CPU16::op_cbw() {
    Operand dst = fetch_operand();
    int8_t byte_val = static_cast<int8_t>(get_value(dst) & 0xFF);
    int16_t word_val = static_cast<int16_t>(byte_val);

    set_value(dst, static_cast<uint16_t>(word_val));
}

void CPU16::op_not() {
    Operand dst = fetch_operand();
    set_value(dst, ~get_value(dst));
}

void CPU16::op_neg() {
    Operand dst = fetch_operand();
    set_value(dst, ~get_value(dst) + 1);
}

void CPU16::op_inc() {
    Operand dst = fetch_operand();
    uint16_t v = get_value(dst) + 1;
    set_value(dst, v);
    set_flag(ZF_MASK, v == 0);
}

void CPU16::op_dec() {
    Operand dst = fetch_operand();
    uint16_t v = get_value(dst) - 1;
    set_value(dst, v);
    set_flag(ZF_MASK, v == 0);
}

void CPU16::op_cmp() {
    uint16_t v1 = get_value(fetch_operand());
    uint16_t v2 = get_value(fetch_operand());
    set_flag(ZF_MASK, v1 == v2);
    set_flag(NF_MASK, static_cast<int16_t>(v1) < static_cast<int16_t>(v2));
}

void CPU16::op_test() {
    uint16_t r = get_value(fetch_operand()) & get_value(fetch_operand());
    set_flag(ZF_MASK, r == 0);
    set_flag(NF_MASK, (r & 0x8000) != 0);
    set_flag(CF_MASK, false);
}

void CPU16::op_jmp() {
    pc = get_value(fetch_operand());
}

void CPU16::op_jmpe() {
    uint16_t t = get_value(fetch_operand());
    if (get_flag(ZF_MASK)) pc = t;
}

void CPU16::op_jmpne() {
    uint16_t t = get_value(fetch_operand());
    if (!get_flag(ZF_MASK)) pc = t;
}

void CPU16::op_jmpg() {
    uint16_t t = get_value(fetch_operand());
    if (!get_flag(NF_MASK) && !get_flag(ZF_MASK)) pc = t;
}

void CPU16::op_jmpl() {
    uint16_t t = get_value(fetch_operand());
    if (get_flag(NF_MASK)) pc = t;
}

void CPU16::op_jmpge() {
    uint16_t t = get_value(fetch_operand());
    if (!get_flag(NF_MASK)) pc = t;
}

void CPU16::op_jmple() {
    uint16_t t = get_value(fetch_operand());
    if (get_flag(NF_MASK) || get_flag(ZF_MASK)) pc = t;
}

void CPU16::op_push() {
    reg[SP]--;
    mem.at(reg[SP]) = get_value(fetch_operand());
}

void CPU16::op_pop() {
    set_value(fetch_operand(), mem.at(reg[SP]++));
}

void CPU16::op_pushf() {
    reg[SP]--;
    mem.at(reg[SP]) = flags;
}

void CPU16::op_popf() {
    flags = mem.at(reg[SP]++);
}

void CPU16::op_pusha() {
    for (int i = 0; i < REG_COUNT; ++i) {
        if (i == SP) continue;
        reg[SP]--;
        mem.at(reg[SP]) = static_cast<uint16_t>(reg[i]);
    }
}

void CPU16::op_popa() {
    for (int i = REG_COUNT - 1; i >= 0; --i) {
        if (i == SP) continue;
        reg[i] = mem.at(reg[SP]++);
    }
}

void CPU16::op_enter() {
    uint16_t size = get_value(fetch_operand());
    reg[SP]--;
    mem.at(reg[SP]) = static_cast<uint16_t>(reg[BP]);
    reg[BP] = reg[SP];
    reg[SP] -= size;
}

void CPU16::op_leave() {
    reg[SP] = reg[BP];
    reg[BP] = mem.at(reg[SP]++);
}

void CPU16::op_call() {
    uint16_t target = get_value(fetch_operand());
    reg[SP]--;
    mem.at(reg[SP]) = static_cast<uint16_t>(pc);
    pc = target;
}

void CPU16::op_ret() {
    pc = mem.at(reg[SP]++);
}

void CPU16::op_int() {
    trigger_interrupt(get_value(fetch_operand()));
}

void CPU16::op_iret() {
    pc = mem.at(reg[SP]++);
    flags = mem.at(reg[SP]++);
}

void CPU16::op_in() {
    Operand dst = fetch_operand();
    uint16_t port = get_value(fetch_operand());
    set_value(dst, peripherals->read_port(port));
}

void CPU16::op_out() {
    uint16_t port = get_value(fetch_operand());
    uint16_t val = get_value(fetch_operand());
    peripherals->write_port(port, val);
}

void CPU16::op_sti() {
    set_flag(IF_MASK, true);
}

void CPU16::op_cli() {
    set_flag(IF_MASK, false);
}

void CPU16::run() {
    while (running) {
        if (pc >= MEM_SIZE) {
            running = false;
            break;
        }
        try {
            Opcode op = static_cast<Opcode>(mem[pc++]);
            switch (op) {
            case Opcode::HLT: op_hlt(); break;
            case Opcode::NOP: op_nop(); break;
            case Opcode::MOV: op_mov(); break;
            case Opcode::XCHG: op_xchg(); break;
            case Opcode::LEA: op_lea(); break;
            case Opcode::ADD: op_add(); break;
            case Opcode::SUB: op_sub(); break;
            case Opcode::MUL: op_mul(); break;
            case Opcode::DIV: op_div(); break;
            case Opcode::MOD: op_mod(); break;
            case Opcode::AND: op_and(); break;
            case Opcode::OR: op_or(); break;
            case Opcode::XOR: op_xor(); break;
            case Opcode::SHL: op_shl(); break;
            case Opcode::SHR: op_shr(); break;
            case Opcode::CBW: op_cbw(); break;
            case Opcode::NOT: op_not(); break;
            case Opcode::INC: op_inc(); break;
            case Opcode::DEC: op_dec(); break;
            case Opcode::CMP: op_cmp(); break;
            case Opcode::JMP: op_jmp(); break;
            case Opcode::JMPE: op_jmpe(); break;
            case Opcode::JMPNE: op_jmpne(); break;
            case Opcode::JMPG: op_jmpg(); break;
            case Opcode::JMPL: op_jmpl(); break;
            case Opcode::JMPGE: op_jmpge(); break;
            case Opcode::JMPLE: op_jmple(); break;
            case Opcode::PUSH: op_push(); break;
            case Opcode::POP: op_pop(); break;
            case Opcode::PUSHF: op_pushf(); break;
            case Opcode::POPF: op_popf(); break;
            case Opcode::PUSHA: op_pusha(); break;
            case Opcode::POPA: op_popa(); break;
            case Opcode::ENTER: op_enter(); break;
            case Opcode::LEAVE: op_leave(); break;
            case Opcode::NEG: op_neg(); break;
            case Opcode::TEST: op_test(); break;
            case Opcode::CALL: op_call(); break;
            case Opcode::RET: op_ret(); break;
            case Opcode::INT: op_int(); break;
            case Opcode::IRET: op_iret(); break;
            case Opcode::IN: op_in(); break;
            case Opcode::OUT: op_out(); break;
            case Opcode::STI: op_sti(); break;
            case Opcode::CLI: op_cli(); break;
            default: trigger_interrupt(1); break;
            }
        }
        catch (const std::runtime_error&) {
            throw;
        }
    }
}

void CPU16::halt() {
    running = false;
}