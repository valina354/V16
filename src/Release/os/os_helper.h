#ifndef OS_HELPER_H
#define OS_HELPER_H

extern char char_to_print;
extern int kb_result;

static inline void putc(char c)
{
    char_to_print = c;

    asm("MOV R0, 1");
    asm("LEA R1, char_to_print");
    asm("MOV R2, [R1]");
    asm("INT 0x10");
}

static inline char getc(void)
{
    asm("MOV R0, 0");
    asm("INT 0x11");

    asm("LEA R1, kb_result");
    asm("MOV [R1], R2");

    return (char)(kb_result & 0xFF);
}

static inline void cls(void)
{
    asm("MOV R0, 4");
    asm("MOV R2, 0x07");
    asm("INT 0x10");
}

static inline void print(const char *s)
{
    int i = 0;
    while (s[i] != 0)
    {
        putc(s[i]);
        i = i + 1;
    }
}

static inline void print_hex(int value)
{
    char hex_chars[] = "0123456789ABCDEF";

    print("0x");

    int started = 0;
    int shift = 28;

    while (shift >= 0)
    {
        int nibble = (value >> shift) & 0xF;

        if (nibble != 0 || started || shift == 0)
        {
            putc(hex_chars[nibble]);
            started = 1;
        }

        shift -= 4;
    }
}

static inline int cmd_is(const char *input, const char *match)
{
    int i = 0;

    while (1)
    {
        int c1 = input[i];
        int c2 = match[i];

        if (c1 >= 'a' && c1 <= 'z')
        {
            c1 -= 32;
        }

        if (c1 != c2) return 0;
        if (c1 == 0) return 1;

        i++;
    }
}

#endif