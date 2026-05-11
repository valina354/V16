#include "os_helper.h"

#define CMD_BUFFER_SIZE 64

#define GDT_REGION_COUNT 16
#define GDT_STRIDE 3 

#define GDT_CODE_BASE    0x0000
#define GDT_CODE_LIMIT   0x7FFF
#define GDT_CODE_ATTR    3

#define GDT_USER_BASE    0x8000
#define GDT_USER_LIMIT   0x0FFF
#define GDT_USER_ATTR    1

char cmd_buffer[CMD_BUFFER_SIZE];
int cmd_ptr = 0;

int verify_result;

/* 2 regions */
int gdt_table[GDT_REGION_COUNT * GDT_STRIDE];

int cpu_status_word = 0;

void drop_to_user(void);

void setup_gdt(void)
{
    int i;
    for (i = 0; i < 48; i++) {
        gdt_table[i] = 0;
    }

    /*
        Region 0:
        Global Code + Stack
        0x0000 - 0x7FFF
        Attr = 3 (Read/Write)
    */

    gdt_table[0] = GDT_CODE_BASE;
    gdt_table[1] = GDT_CODE_LIMIT;
    gdt_table[2] = GDT_CODE_ATTR;

    /*
        Region 1:
        Read-Only User Data
        0x8000 - 0x8FFF
        Attr = 1 (Read Only)
    */

    gdt_table[3] = GDT_USER_BASE;
    gdt_table[4] = GDT_USER_LIMIT;
    gdt_table[5] = GDT_USER_ATTR;

    asm("LEA R0, gdt_table");
    asm("LGDT R0");
}

void enter_pm(void)
{
    setup_gdt();

    asm("MOV R0, 1");
    asm("LMSW R0");

    print("Protected Mode Enabled\n");
	
	drop_to_user();
}

void drop_to_user(void)
{
    print("Switching To Ring 3...\n");

    asm("PUSHF");

    asm("LEA R0, user_land");
    asm("PUSH R0");

    asm("MOV R0, 3");
    asm("PUSH R0");

    asm("IRET");

    asm("user_land:");
}

// Test VERW
void verify_ro_segment(void)
{
    print("Checking RO Segment @ 0x8050...\n");

    asm("MOV R1, 0x8050");
    asm("VERW R0, R1");

    asm("LEA R1, verify_result");
    asm("MOV [R1], R0");

    if (verify_result == 0)
    {
        print("Result: PROTECTED\n");
    }
    else
    {
        print("Result: FAILED\n");
    }
}

void show_status(void)
{
    print("Reading MSW...\n");

    asm("SMSW R0");
    asm("LEA R1, cpu_status_word");
    asm("MOV [R1], R0");

    print("MSW = ");
    print_hex(cpu_status_word);
    putc(10);
}

// Intentionally triggers GPF
void gpf(void)
{
    asm("HLT");
}

// Triggers int3 which prints out cpu details
void dbg(void)
{
    asm("INT3");
}

void show_prompt(void)
{
    print("> ");
}

void process_command(void)
{
    putc(10);

    if (cmd_is(cmd_buffer, "VERIFY"))
    {
        verify_ro_segment();
    }
    else if (cmd_is(cmd_buffer, "STATUS"))
    {
        show_status();
    }
    else if (cmd_is(cmd_buffer, "GPF"))
    {
        gpf();
    }
    else if (cmd_is(cmd_buffer, "DBG"))
    {
        dbg();
    }
    else
    {
        print("Commands: VERIFY, STATUS, GPF, DBG\n");
    }

    cmd_ptr = 0;
    show_prompt();
}

/* ---------- Main ---------- */

int main(void)
{
    cls();
    enter_pm();

    print("V-DOS v1.0\n");
    print("System Ready\n");

    show_prompt();

    while (1)
    {
        char c = getc();

        if (c == 13)
        {
            cmd_buffer[cmd_ptr] = 0;
            process_command();
        }
        else if (c == 8)
        {
            if (cmd_ptr > 0)
            {
                cmd_ptr--;
                putc(8);
            }
        }
        else if (c >= 32 && cmd_ptr < (CMD_BUFFER_SIZE - 1))
        {
            cmd_buffer[cmd_ptr++] = c;
            putc(c);
        }
    }

    return 0;
}