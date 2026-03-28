    JMP main
    ; Data Section
.L..17:
    DB 55
    DB 45
    DB 49
    DB 48
    DB 32
    DB 105
    DB 115
    DB 32
    DB 0
.L..16:
    DB 10
    DB 0
.L..15:
    DB 50
    DB 43
    DB 55
    DB 32
    DB 105
    DB 115
    DB 32
    DB 0
.L..14:
    DB 10
    DB 72
    DB 101
    DB 108
    DB 108
    DB 111
    DB 32
    DB 102
    DB 114
    DB 111
    DB 109
    DB 32
    DB 67
    DB 33
    DB 10
    DB 0
.L..13:
    DB 109
    DB 97
    DB 105
    DB 110
    DB 0
.L..12:
    DB 109
    DB 97
    DB 105
    DB 110
    DB 0
.L..7:
    DB 112
    DB 114
    DB 105
    DB 110
    DB 116
    DB 95
    DB 110
    DB 117
    DB 109
    DB 98
    DB 101
    DB 114
    DB 0
.L..6:
    DB 112
    DB 114
    DB 105
    DB 110
    DB 116
    DB 95
    DB 110
    DB 117
    DB 109
    DB 98
    DB 101
    DB 114
    DB 0
.L..3:
    DB 112
    DB 114
    DB 105
    DB 110
    DB 116
    DB 95
    DB 115
    DB 116
    DB 114
    DB 105
    DB 110
    DB 103
    DB 0
.L..2:
    DB 112
    DB 114
    DB 105
    DB 110
    DB 116
    DB 95
    DB 115
    DB 116
    DB 114
    DB 105
    DB 110
    DB 103
    DB 0
.L..1:
    DB 112
    DB 114
    DB 105
    DB 110
    DB 116
    DB 95
    DB 99
    DB 104
    DB 97
    DB 114
    DB 0
.L..0:
    DB 112
    DB 114
    DB 105
    DB 110
    DB 116
    DB 95
    DB 99
    DB 104
    DB 97
    DB 114
    DB 0
num_to_print:
    DW 0 ; zero-initialized
char_to_print:
    DW 0 ; zero-initialized
    ; Text Section

; Function: main
main:
    ENTER 150
    PUSH R4
    PUSH R5
    PUSH R6
    PUSH R7
    PUSH R8
    PUSH R9
    PUSH R10
    PUSH R11
    MOV R0, .L..14
    PUSH R0
    POP R0
    CALL print_string
    LEA R0, [BP - 12]
    MOV R1, 0
    MOV R2, R0
    MOV [R2], R1
    LEA R0, [BP - 12]
    PUSH R0
    MOV R0, 2
    POP R1
    MOV [R1], R0
    LEA R0, [BP - 10]
    MOV R1, 0
    MOV R2, R0
    MOV [R2], R1
    LEA R0, [BP - 10]
    PUSH R0
    MOV R0, 7
    POP R1
    MOV [R1], R0
    LEA R0, [BP - 8]
    MOV R1, 0
    MOV R2, R0
    MOV [R2], R1
    LEA R0, [BP - 8]
    PUSH R0
    LEA R0, [BP - 10]
    MOV R0, [R0]
    PUSH R0
    LEA R0, [BP - 12]
    MOV R0, [R0]
    POP R1
    ADD R0, R1
    POP R1
    MOV [R1], R0
    MOV R0, .L..15
    PUSH R0
    POP R0
    CALL print_string
    LEA R0, [BP - 8]
    MOV R0, [R0]
    PUSH R0
    POP R0
    CALL print_number
    MOV R0, .L..16
    PUSH R0
    POP R0
    CALL print_string
    LEA R0, [BP - 6]
    MOV R1, 0
    MOV R2, R0
    MOV [R2], R1
    LEA R0, [BP - 6]
    PUSH R0
    MOV R0, 7
    POP R1
    MOV [R1], R0
    LEA R0, [BP - 4]
    MOV R1, 0
    MOV R2, R0
    MOV [R2], R1
    LEA R0, [BP - 4]
    PUSH R0
    MOV R0, 10
    POP R1
    MOV [R1], R0
    LEA R0, [BP - 2]
    MOV R1, 0
    MOV R2, R0
    MOV [R2], R1
    LEA R0, [BP - 2]
    PUSH R0
    LEA R0, [BP - 4]
    MOV R0, [R0]
    PUSH R0
    LEA R0, [BP - 6]
    MOV R0, [R0]
    POP R1
    SUB R0, R1
    POP R1
    MOV [R1], R0
    MOV R0, .L..17
    PUSH R0
    POP R0
    CALL print_string
    LEA R0, [BP - 2]
    MOV R0, [R0]
    PUSH R0
    POP R0
    CALL print_number
.L.begin.1:
    MOV R0, 1
    CMP R0, 0
    JMPE .L..18
.L..19:
    JMP .L.begin.1
.L..18:
    MOV R0, 0
    JMP .L.return.main
.L.return.main:
    POP R11
    POP R10
    POP R9
    POP R8
    POP R7
    POP R6
    POP R5
    POP R4
    LEAVE
    RET

; Function: print_number
print_number:
    ENTER 30
    PUSH R4
    PUSH R5
    PUSH R6
    PUSH R7
    PUSH R8
    PUSH R9
    PUSH R10
    PUSH R11
    MOV [BP - 30], R0
    LEA R0, [BP - 10]
    MOV R1, 0
    MOV R2, R0
    MOV [R2], R1
    LEA R0, [BP - 10]
    PUSH R0
    MOV R0, 0
    POP R1
    MOV [R1], R0
    MOV R0, 0
    PUSH R0
    LEA R0, [BP - 30]
    MOV R0, [R0]
    POP R1
    CMP R0, R1
    JMPE .L.true.3
    MOV R0, 0
    JMP .L.end.3
.L.true.3:
    MOV R0, 1
.L.end.3:
    CMP R0, 0
    JMPE .L.else.2
    MOV R0, 48
    CBW R0
    PUSH R0
    POP R0
    CALL print_char
    JMP .L.return.print_number
    JMP .L.end.2
.L.else.2:
.L.end.2:
    MOV R0, 0
    PUSH R0
    LEA R0, [BP - 30]
    MOV R0, [R0]
    POP R1
    CMP R0, R1
    JMPL .L.true.5
    MOV R0, 0
    JMP .L.end.5
.L.true.5:
    MOV R0, 1
.L.end.5:
    CMP R0, 0
    JMPE .L.else.4
    MOV R0, 45
    CBW R0
    PUSH R0
    POP R0
    CALL print_char
    LEA R0, [BP - 30]
    PUSH R0
    LEA R0, [BP - 30]
    MOV R0, [R0]
    NEG R0
    POP R1
    MOV [R1], R0
    JMP .L.end.4
.L.else.4:
.L.end.4:
.L.begin.6:
    LEA R0, [BP - 30]
    MOV R0, [R0]
    PUSH R0
    MOV R0, 0
    POP R1
    CMP R0, R1
    JMPL .L.true.7
    MOV R0, 0
    JMP .L.end.7
.L.true.7:
    MOV R0, 1
.L.end.7:
    CMP R0, 0
    JMPE .L..8
    LEA R0, [BP - 8]
    MOV R1, 0
    MOV R2, R0
    MOV [R2], R1
    LEA R0, [BP - 8]
    PUSH R0
    MOV R0, 10
    PUSH R0
    LEA R0, [BP - 30]
    MOV R0, [R0]
    POP R1
    MOD R0, R1
    POP R1
    MOV [R1], R0
    MOV R0, 1
    PUSH R0
    MOV R0, -1
    PUSH R0
    LEA R0, [BP - 6]
    PUSH R0
    LEA R0, [BP - 10]
    POP R1
    MOV [R1], R0
    LEA R0, [BP - 6]
    MOV R0, [R0]
    PUSH R0
    MOV R0, 1
    PUSH R0
    LEA R0, [BP - 6]
    MOV R0, [R0]
    MOV R0, [R0]
    POP R1
    ADD R0, R1
    POP R1
    MOV [R1], R0
    POP R1
    ADD R0, R1
    POP R1
    MUL R0, R1
    PUSH R0
    LEA R0, [BP - 26]
    POP R1
    ADD R0, R1
    PUSH R0
    LEA R0, [BP - 8]
    MOV R0, [R0]
    PUSH R0
    MOV R0, 48
    POP R1
    ADD R0, R1
    CBW R0
    POP R1
    MOV [R1], R0
    LEA R0, [BP - 4]
    PUSH R0
    LEA R0, [BP - 30]
    POP R1
    MOV [R1], R0
    LEA R0, [BP - 4]
    MOV R0, [R0]
    PUSH R0
    MOV R0, 10
    PUSH R0
    LEA R0, [BP - 4]
    MOV R0, [R0]
    MOV R0, [R0]
    POP R1
    DIV R0, R1
    POP R1
    MOV [R1], R0
.L..9:
    JMP .L.begin.6
.L..8:
.L.begin.8:
    LEA R0, [BP - 10]
    MOV R0, [R0]
    PUSH R0
    MOV R0, 0
    POP R1
    CMP R0, R1
    JMPL .L.true.9
    MOV R0, 0
    JMP .L.end.9
.L.true.9:
    MOV R0, 1
.L.end.9:
    CMP R0, 0
    JMPE .L..10
    MOV R0, 1
    PUSH R0
    LEA R0, [BP - 2]
    PUSH R0
    LEA R0, [BP - 10]
    POP R1
    MOV [R1], R0
    LEA R0, [BP - 2]
    MOV R0, [R0]
    PUSH R0
    MOV R0, 1
    PUSH R0
    LEA R0, [BP - 2]
    MOV R0, [R0]
    MOV R0, [R0]
    POP R1
    SUB R0, R1
    POP R1
    MOV [R1], R0
    POP R1
    MUL R0, R1
    PUSH R0
    LEA R0, [BP - 26]
    POP R1
    ADD R0, R1
    MOV R0, [R0]
    CBW R0
    PUSH R0
    POP R0
    CALL print_char
.L..11:
    JMP .L.begin.8
.L..10:
.L.return.print_number:
    POP R11
    POP R10
    POP R9
    POP R8
    POP R7
    POP R6
    POP R5
    POP R4
    LEAVE
    RET

; Function: print_string
print_string:
    ENTER 8
    PUSH R4
    PUSH R5
    PUSH R6
    PUSH R7
    PUSH R8
    PUSH R9
    PUSH R10
    PUSH R11
    MOV [BP - 8], R0
    LEA R0, [BP - 4]
    MOV R1, 0
    MOV R2, R0
    MOV [R2], R1
    LEA R0, [BP - 4]
    PUSH R0
    MOV R0, 0
    POP R1
    MOV [R1], R0
.L.begin.10:
    MOV R0, 0
    PUSH R0
    MOV R0, 1
    PUSH R0
    LEA R0, [BP - 4]
    MOV R0, [R0]
    POP R1
    MUL R0, R1
    PUSH R0
    LEA R0, [BP - 8]
    MOV R0, [R0]
    POP R1
    ADD R0, R1
    MOV R0, [R0]
    POP R1
    CMP R0, R1
    JMPNE .L.true.11
    MOV R0, 0
    JMP .L.end.11
.L.true.11:
    MOV R0, 1
.L.end.11:
    CMP R0, 0
    JMPE .L..4
    MOV R0, 1
    PUSH R0
    LEA R0, [BP - 4]
    MOV R0, [R0]
    POP R1
    MUL R0, R1
    PUSH R0
    LEA R0, [BP - 8]
    MOV R0, [R0]
    POP R1
    ADD R0, R1
    MOV R0, [R0]
    CBW R0
    PUSH R0
    POP R0
    CALL print_char
    MOV R0, -1
    PUSH R0
    LEA R0, [BP - 2]
    PUSH R0
    LEA R0, [BP - 4]
    POP R1
    MOV [R1], R0
    LEA R0, [BP - 2]
    MOV R0, [R0]
    PUSH R0
    MOV R0, 1
    PUSH R0
    LEA R0, [BP - 2]
    MOV R0, [R0]
    MOV R0, [R0]
    POP R1
    ADD R0, R1
    POP R1
    MOV [R1], R0
    POP R1
    ADD R0, R1
.L..5:
    JMP .L.begin.10
.L..4:
.L.return.print_string:
    POP R11
    POP R10
    POP R9
    POP R8
    POP R7
    POP R6
    POP R5
    POP R4
    LEAVE
    RET

; Function: print_char
print_char:
    ENTER 4
    PUSH R4
    PUSH R5
    PUSH R6
    PUSH R7
    PUSH R8
    PUSH R9
    PUSH R10
    PUSH R11
    MOV [BP - 3], R0
    MOV R0, char_to_print
    PUSH R0
    LEA R0, [BP - 3]
    MOV R0, [R0]
    CBW R0
    POP R1
    MOV [R1], R0
    PUSH R0
    PUSH R2
    MOV R0, 1
    MOV R2, char_to_print
    MOV R2, [R2]
    INT 0x10
    POP R2
    POP R0
.L.return.print_char:
    POP R11
    POP R10
    POP R9
    POP R8
    POP R7
    POP R6
    POP R5
    POP R4
    LEAVE
    RET