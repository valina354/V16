#include "cpu.h"
#include "peripherals.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <iomanip>

#ifdef _WIN32
#undef IN
#undef OUT
#endif

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
    halted = false;
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
        {"STI", Opcode::STI}, {"CLI", Opcode::CLI},
        {"LGDT", Opcode::LGDT}, {"LMSW", Opcode::LMSW}, {"SMSW", Opcode::SMSW}, {"VERR", Opcode::VERR}, {"VERW", Opcode::VERW},
        {"INT3", Opcode::INT3},
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

bool CPU16::validate_access(size_t addr, bool write)
{
    if ((msw & 1) && (cpl != 0))
    {
        if (addr < 0x0400)
        {
            std::cout << "[GPF] IVT Access" << std::endl;
            return false;
        }

        for (const auto& reg : gdt)
        {
            if (addr >= reg.base && addr <= (reg.base + reg.limit))
            {
                bool allowed = (write ? (reg.attr & 2) : (reg.attr & 1));
                if (!allowed)
                {
                    std::cout << "[GPF] Permission mismatch in segment starting at 0x"
                        << std::hex << reg.base << std::dec << std::endl;
                }
                return allowed;
            }
        }

        std::cout << "[GPF] Address outside any GDT segment" << std::endl;
        return false;
    }
    return true;
}

uint16_t CPU16::safe_read(size_t addr)
{
    if (addr >= MEM_SIZE || !validate_access(addr, false))
    {
        trigger_interrupt(2);
        return 0;
    }
    return mem[addr];
}

void CPU16::safe_write(size_t addr, uint16_t value)
{
    if (addr >= MEM_SIZE || !validate_access(addr, true))
    {
        trigger_interrupt(2);
        return;
    }
    mem[addr] = value;
}

bool CPU16::pfault()
{
    if ((msw & 1) && (cpl != 0))
    {
        std::cout << "[GPF] Privileged instruction fault at PC: 0x" << std::hex << pc << std::dec << std::endl;
        trigger_interrupt(2);
        return 0;
    }

    return 1;
}

Operand CPU16::parse_operand_string(const std::string& op_str_raw, const std::unordered_map<std::string, size_t>& labels) {
    std::string s = trim_str(op_str_raw);
    to_upper_str(s);

    if (s.empty()) return { OperandType::NONE, 0, "" };

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
            else if (reg_str.rfind("R", 0) == 0) {
                try { base_reg_num = std::stoi(reg_str.substr(1)); }
                catch (...) { throw std::runtime_error("Invalid register in offset: " + reg_str); }
            }

            int offset = 0;
            try { offset = std::stoi(offset_str, nullptr, 0); }
            catch (...) { throw std::runtime_error("Invalid offset constant: " + offset_str); }

            int32_t packed = (static_cast<int16_t>(offset) << 16) | (base_reg_num & 0xFFFF);
            return { OperandType::MEM_REG_OFFSET, packed, "" };
        }
        try {
            int addr = std::stoi(inner, nullptr, 0);
            return { OperandType::MEM_DIRECT, addr, "" };
        }
        catch (...) {
            int reg_num = -1;
            if (inner == "BP") reg_num = BP;
            else if (inner == "SP") reg_num = SP;
            else if (inner.rfind("R", 0) == 0) reg_num = std::stoi(inner.substr(1));
            return { OperandType::MEM_INDIRECT, reg_num, "" };
        }
    }

    if (s == "BP") return { OperandType::REG, BP, "" };
    if (s == "SP") return { OperandType::REG, SP, "" };

    if (s.length() >= 2 && s[0] == 'R' && std::isdigit(s[1])) {
        try {
            int r_num = std::stoi(s.substr(1));
            return { OperandType::REG, r_num, "" };
        }
        catch (...) {  }
    }

    if (s.length() == 3 && s.front() == '\'' && s.back() == '\'') {
        return { OperandType::IMM, static_cast<int>(s[1]), "" };
    }

    try {
        size_t pos;
        int val = std::stoi(s, &pos, 0);
        if (pos == s.size()) return { OperandType::IMM, val, "" };
    }
    catch (...) {}

    std::string upper_s = s;
    to_upper_str(upper_s);
    if (labels.count(upper_s)) {
        return { OperandType::IMM, static_cast<int>(labels.at(upper_s)), "" };
    }

    return { OperandType::LABEL, 0, s };
}

void CPU16::load_program(const std::string& filename, size_t offset) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + filename);

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        size_t cp = line.find(';');
        if (cp != std::string::npos)
            line = line.substr(0, cp);
        line = trim_str(line);
        if (!line.empty()) 
            lines.push_back(line);
    }

    std::unordered_map<std::string, size_t> labels;

    size_t ptr = offset;
    for (auto& l_orig : lines) {
        std::string l = l_orig;
        size_t cl = l.find(':');
        if (cl != std::string::npos) {
            std::string n = trim_str(l.substr(0, cl));
            to_upper_str(n);
            labels[n] = ptr;
            l = trim_str(l.substr(cl + 1));
        }
        if (l.empty()) continue;

        size_t fs = l.find_first_of(" \t");
        std::string op = l.substr(0, fs);
        to_upper_str(op);

        if (op == "DB") {
            std::string args = trim_str(l.substr(op.length()));
            bool in_s = false; args += ','; std::string seg;
            for (char c : args) {
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
            std::string args = trim_str(l.substr(op.length()));
            std::stringstream ss(args); std::string t;
            while (std::getline(ss, t, ',')) ptr++;
        }
        else {
            ptr++;
            size_t fs_idx = l.find_first_of(" \t");
            if (fs_idx != std::string::npos) {
                std::string args = trim_str(l.substr(fs_idx + 1));
                std::stringstream ss(args); std::string a;
                while (std::getline(ss, a, ',')) ptr += 3;
            }
        }
    }

    ptr = offset;
    for (const auto& l_orig : lines) {
        std::string l = l_orig;
        try {
            size_t cl = l.find(':');
            if (cl != std::string::npos) l = trim_str(l.substr(cl + 1));
            if (l.empty()) continue;

            size_t fs = l.find_first_of(" \t");
            std::string op_s = l.substr(0, fs);
            std::string args = (fs != std::string::npos) ? trim_str(l.substr(fs + 1)) : "";
            to_upper_str(op_s);

            if (op_s == "DB") {
                bool in_s = false; std::string seg; args += ',';
                for (char c : args) {
                    if (c == '"') in_s = !in_s;
                    if (c == ',' && !in_s) {
                        seg = trim_str(seg);
                        if (!seg.empty()) {
                            if (seg.front() == '"' || seg.front() == '\'') {
                                for (size_t i = 1; i < seg.length() - 1; ++i) mem[ptr++] = (uint8_t)seg[i];
                            }
                            else {
                                mem[ptr++] = (uint8_t)std::stoi(seg, nullptr, 0);
                            }
                        }
                        seg = "";
                    }
                    else seg += c;
                }
            }
            else if (op_s == "DW") {
                std::stringstream ss(args); std::string t;
                while (std::getline(ss, t, ',')) {
                    std::string item = trim_str(t);
                    to_upper_str(item);
                    if (labels.count(item)) mem[ptr++] = (uint16_t)labels[item];
                    else mem[ptr++] = (uint16_t)std::stoi(item, nullptr, 0);
                }
            }
            else {
                mem[ptr++] = (uint16_t)(opcodes.count(op_s) ? opcodes.at(op_s) : Opcode::INVALID);
                if (!args.empty()) {
                    std::stringstream ss(args); std::string a;
                    while (std::getline(ss, a, ',')) {
                        Operand o = parse_operand_string(a, labels);
                        if (o.type == OperandType::LABEL) {
                            std::string un = o.name; to_upper_str(un);
                            if (labels.count(un)) { o.value = (int)labels.at(un); o.type = OperandType::IMM; }
                            else throw std::runtime_error("Undefined label: " + un);
                        }
                        mem[ptr++] = (uint16_t)o.type;
                        mem[ptr++] = (uint16_t)(o.value & 0xFFFF);
                        mem[ptr++] = (uint16_t)((o.value >> 16) & 0xFFFF);
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Assembler Error: " << e.what() << " on line: " << l_orig << std::endl;
            throw;
        }
    }
}

uint16_t CPU16::get_value(const Operand& operand) {
    switch (operand.type) {
    case OperandType::REG:
        return reg.at(operand.value) & 0xFFFF;
    case OperandType::IMM:
        return operand.value & 0xFFFF;
    case OperandType::MEM_DIRECT: 
        return safe_read(operand.value);
    case OperandType::MEM_INDIRECT: {
        return safe_read(reg.at(operand.value));
    }
    case OperandType::MEM_REG_OFFSET: {
        uint16_t base_reg = operand.value & 0xFFFF;
        int16_t offset = operand.value >> 16;

        return safe_read(reg.at(base_reg) + offset);
    }
    default: throw std::runtime_error("Invalid get_value");
    }
}

void CPU16::set_value(const Operand& operand, uint16_t value) {
    switch (operand.type) {
    case OperandType::REG:
        reg.at(operand.value) = value;
        break;
    case OperandType::MEM_DIRECT: 
        safe_write(operand.value, value); break;
    case OperandType::MEM_INDIRECT: {
        safe_write(reg.at(operand.value), value);
        break;
    }
    case OperandType::MEM_REG_OFFSET: {
        uint16_t base_reg = operand.value & 0xFFFF;
        int16_t offset = operand.value >> 16;
        safe_write(reg.at(base_reg) + offset, value);
        break;
    }
    default: throw std::runtime_error("Invalid set_value");
    }
}

void CPU16::trigger_interrupt(uint16_t vector_num) {
    if (!get_flag(IF_MASK)) 
        return;

    halted = false;
    uint16_t old_cpl = cpl;
    cpl = 0;

    reg[SP]--;
    safe_write(reg[SP], flags);
    reg[SP]--;
    safe_write(reg[SP], static_cast<uint16_t>(pc));

    if (msw & 1) {
        reg[SP]--;
        safe_write(reg[SP], old_cpl);
    }

    set_flag(IF_MASK, false);
    pc = safe_read(IVT_ADDRESS + vector_num);
}

Operand CPU16::fetch_operand() {
    Operand op;
    op.type = static_cast<OperandType>(safe_read(pc++));
    uint16_t low = safe_read(pc++);
    uint16_t high = safe_read(pc++);
    op.value = (static_cast<uint32_t>(high) << 16) | low;
    return op;
}

void CPU16::op_hlt() {
    if (!pfault())
        return;

    halted = true;
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
    case OperandType::MEM_DIRECT:
        effective_address = src.value & 0xFFFF;
        break;

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
    uint16_t val = get_value(fetch_operand());
    reg[SP]--;
    safe_write(reg[SP], val);
}

void CPU16::op_pop() {
    set_value(fetch_operand(), safe_read(reg[SP]++));
}

void CPU16::op_pushf() {
    reg[SP]--;
    safe_write(reg[SP], flags);
}

void CPU16::op_popf() {
    flags = safe_read(reg[SP]++);
}

void CPU16::op_pusha() {
    uint32_t sp_val = reg[SP];
    for (int i = 0; i < REG_COUNT; ++i) {
        if (i == SP) continue;
        reg[SP]--;
        safe_write(reg[SP], static_cast<uint16_t>(reg[i]));
    }
}

void CPU16::op_popa() {
    for (int i = REG_COUNT - 1; i >= 0; --i) {
        if (i == SP) continue;
        reg[i] = safe_read(reg[SP]++);
    }
}

void CPU16::op_enter() {
    uint16_t size = get_value(fetch_operand());
    reg[SP]--;
    safe_write(reg[SP], static_cast<uint16_t>(reg[BP]));
    reg[BP] = reg[SP];
    reg[SP] -= size;
}

void CPU16::op_leave() {
    reg[SP] = reg[BP];
    reg[BP] = safe_read(reg[SP]++);
}

void CPU16::op_call() {
    uint16_t target = get_value(fetch_operand());
    reg[SP]--;
    safe_write(reg[SP], static_cast<uint16_t>(pc));
    pc = target;
}

void CPU16::op_ret() {
    pc = safe_read(reg[SP]++);
}

void CPU16::op_int() {
    trigger_interrupt(get_value(fetch_operand()));
}

void CPU16::op_int3() {
    std::cout << "\n--- DEBUG BREAKPOINT ---" << std::endl;
    std::cout << "PC:   0x" << std::hex << std::setw(4) << std::setfill('0') << pc << std::dec << std::endl;
    std::cout << "SP:   0x" << std::hex << std::setw(4) << std::setfill('0') << reg[SP] << std::dec << std::endl;
    std::cout << "MSW:  " << msw << " (PE=" << (msw & 1) << ")" << std::endl;
    std::cout << "CPL:  " << cpl << std::endl;

    std::cout << "\n[GDTR - " << gdt.size() << " Active Regions]" << std::endl;
    if (gdt.empty()) {
        std::cout << "  (Table is empty)" << std::endl;
    }
    else {
        for (size_t i = 0; i < gdt.size(); i++) {
            const auto& r = gdt[i];
            std::cout << "  Region " << i << ": Base=0x" << std::hex << r.base
                << " Limit=0x" << r.limit
                << " Attr=0x" << r.attr
                << " (" << ((r.attr & 1) ? "R" : "-")
                << ((r.attr & 2) ? "W" : "-") << ")"
                << std::dec << std::endl;
        }
    }

    std::cout << "------------------------------" << std::endl;
}

void CPU16::op_iret() {
    if (!pfault())
        return;

    uint16_t popped_cpl = cpl;
    uint16_t old_sp = reg[SP];

    if (msw & 1) {
        popped_cpl = safe_read(reg[SP]++);
        cpl = popped_cpl;
    }

    uint16_t popped_pc = safe_read(reg[SP]++);
    uint16_t popped_flags = safe_read(reg[SP]++);

    pc = popped_pc;
    flags = popped_flags;
}

void CPU16::op_in() {
    if (!pfault())
        return;

    Operand dst = fetch_operand();
    uint16_t port = get_value(fetch_operand());
    set_value(dst, peripherals->read_port(port));
}

void CPU16::op_out() {
    if (!pfault())
        return;

    uint16_t port = get_value(fetch_operand());
    uint16_t val = get_value(fetch_operand());
    peripherals->write_port(port, val);
}

void CPU16::op_sti() {
    if (!pfault())
        return;

    set_flag(IF_MASK, true);
}

void CPU16::op_cli() {
    if (!pfault())
        return;

    set_flag(IF_MASK, false);
}

void CPU16::op_lgdt()
{
    if (!pfault())
        return;

    uint32_t addr = get_value(fetch_operand());

    gdt.clear();
    for (int i = 0; i < 16; i++) // Max of 16 GDT
    {
        uint32_t base = (uint32_t)safe_read(addr + (i * 6)) | ((uint32_t)safe_read(addr + (i * 6) + 1) << 16);
        uint16_t limit = safe_read(addr + (i * 6) + 2);
        uint16_t attr = safe_read(addr + (i * 6) + 4);

        gdt.push_back({ base, limit, attr });
    }
}

void CPU16::op_lmsw() {
    if (!pfault())
        return;

    msw = get_value(fetch_operand());
}

void CPU16::op_smsw() {
    set_value(fetch_operand(), msw);
}

void CPU16::op_verr() {
    Operand dst = fetch_operand();
    set_value(dst, validate_access(get_value(fetch_operand()), false) ? 1 : 0);
}

void CPU16::op_verw() {
    Operand dst = fetch_operand();
    set_value(dst, validate_access(get_value(fetch_operand()), true) ? 1 : 0);
}

void CPU16::run() {
    while (running) {
        if (halted) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        if (pc >= MEM_SIZE) {
            running = false;
            break;
        }
        try {
            Opcode op = static_cast<Opcode>(safe_read(pc++));

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
            case Opcode::INT3: op_int3(); break;
            case Opcode::IRET: op_iret(); break;
            case Opcode::IN: op_in(); break;
            case Opcode::OUT: op_out(); break;
            case Opcode::STI: op_sti(); break;
            case Opcode::CLI: op_cli(); break;
            case Opcode::LGDT: op_lgdt(); break;
            case Opcode::LMSW: op_lmsw(); break;
            case Opcode::SMSW: op_smsw(); break;
            case Opcode::VERR: op_verr(); break;
            case Opcode::VERW: op_verw(); break;
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