// bios specific func calls
char char_to_print;
char kb_result;

void print_char(char c) {
    char_to_print = c;

    asm("PUSH R0");
    asm("PUSH R2");
    asm("MOV R0, 1");
    asm("MOV R2, char_to_print");
    asm("MOV R2, [R2]");
    asm("INT 0x10");
    asm("POP R2");
    asm("POP R0");
}

char get_char() {
    asm("PUSH R0");
    asm("MOV R0, 0");
    asm("INT 0x11");
    asm("MOV R1, kb_result");
    asm("MOV [R1], R2");
    asm("POP R0");

    return kb_result;
}
// end bios specific func calls

void print_string(char *str) {
    int i = 0;
    while (str[i] != 0) {
        print_char(str[i]);
        i++;
    }
}

void print_number(int n) {
    char buffer[16];
    int i = 0;

    if (n == 0) {
        print_char('0');
        return;
    }

    if (n < 0) {
        print_char('-');
        n = -n;
    }

    while (n > 0) {
        int digit = n % 10;
        buffer[i++] = '0' + digit;
        n /= 10;
    }

    while (i > 0) {
        print_char(buffer[--i]);
    }
}

int main() {
    print_string("\nTypewriter.\n");

    while(1) {
        char c = get_char();

        if (c == 13) {
            print_char(10);
        } else {
            print_char(c);
        }
    }
}