#ifndef CPU16_H
#define CPU16_H

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <atomic>

class Peripherals;

const size_t MEM_SIZE = 16 * 1024 * 1024;
const size_t REG_COUNT = 16;
const int SP = 15;
const int BP = 14;

const uint16_t IVT_ADDRESS = 0x0000;

const uint16_t CF_MASK = 1 << 0;
const uint16_t ZF_MASK = 1 << 1;
const uint16_t NF_MASK = 1 << 2;
const uint16_t IF_MASK = 1 << 3;

enum class Opcode : uint16_t {
    HLT, NOP, MOV, XCHG, LEA, ADD, SUB, MUL, DIV, MOD, AND, OR, XOR, SHL, SHR, CBW,
    NOT, INC, DEC, CMP, JMP, JMPE, JMPNE, JMPG, JMPL, JMPGE, JMPLE,
    PUSH, POP, PUSHF, POPF, PUSHA, POPA, ENTER, LEAVE, NEG, TEST, CALL, RET, INT, IRET,
    IN, OUT, STI, CLI,
    INVALID
};

enum class OperandType : uint16_t {
    REG, IMM, LABEL,
    MEM_INDIRECT,
    MEM_REG_OFFSET,
    NONE
};

struct Operand {
    OperandType type;
    int value = 0;
    std::string name;
};

class CPU16 {
private:
    std::vector<uint16_t> mem;
    std::vector<uint32_t> reg;
    size_t pc;
    std::atomic<bool> running;
    uint16_t flags;
    std::shared_ptr<Peripherals> peripherals;
    std::unordered_map<std::string, Opcode> opcodes;

    void set_flag(uint16_t mask, bool value);
    bool get_flag(uint16_t mask) const;

    Operand parse_operand_string(const std::string& op_str_raw, const std::unordered_map<std::string, size_t>& labels);
    uint16_t get_value(const Operand& operand);
    void set_value(const Operand& operand, uint16_t value);
    void trigger_interrupt(uint16_t vector_num);
    Operand fetch_operand();

    void op_hlt();
    void op_nop();
    void op_mov();
    void op_xchg();
    void op_lea();
    void op_add();
    void op_sub();
    void op_mul();
    void op_div();
    void op_mod();
    void op_and();
    void op_or();
    void op_xor();
    void op_shl();
    void op_shr();
    void op_cbw();
    void op_not();
    void op_neg();
    void op_inc();
    void op_dec();
    void op_cmp();
    void op_test();
    void op_jmp();
    void op_jmpe();
    void op_jmpne();
    void op_jmpg();
    void op_jmpl();
    void op_jmpge();
    void op_jmple();
    void op_push();
    void op_pop();
    void op_pushf();
    void op_popf();
    void op_pusha();
    void op_popa();
    void op_enter();
    void op_leave();
    void op_call();
    void op_ret();
    void op_int();
    void op_iret();
    void op_in();
    void op_out();
    void op_sti();
    void op_cli();

public:
    CPU16(std::shared_ptr<Peripherals> peri);
    void load_program(const std::string& filename, size_t offset);
    void run();
    void halt();
    bool is_running() const;
    void set_pc(size_t address);

    friend class Peripherals;
};

#endif