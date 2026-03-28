// test program written in C build with ./chibicc -S -o test.asm test.c and copy the made asm into program.asm

// bios specific func calls
char char_to_print;
int num_to_print;

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
	print_string("\nHello from C!\n");

	int x1 = 2;
	int y1 = 7;
	int result1 = x1 + y1;

	print_string("2+7 is ");
	print_number(result1);
	print_string("\n");
	
	int x2 = 7;
	int y2 = 10;
	int result2 = x2 - y2;

	print_string("7-10 is ");
	print_number(result2);
	
	while(1){}

    return 0;
}