    JMP main
    ; Data Section
.L..16:
    DB 10
    DB 84
    DB 121
    DB 112
    DB 101
    DB 119
    DB 114
    DB 105
    DB 116
    DB 101
    DB 114
    DB 46
    DB 10
    DB 0
.L..15:
    DB 109
    DB 97
    DB 105
    DB 110
    DB 0
.L..14:
    DB 109
    DB 97
    DB 105
    DB 110
    DB 0
.L..9:
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
.L..8:
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
.L..5:
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
.L..4:
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
.L..3:
    DB 103
    DB 101
    DB 116
    DB 95
    DB 99
    DB 104
    DB 97
    DB 114
    DB 0
.L..2:
    DB 103
    DB 101
    DB 116
    DB 95
    DB 99
    DB 104
    DB 97
    DB 114
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
kb_result:
    DW 0 ; zero-initialized
char_to_print:
    DW 0 ; zero-initialized
    ; Text Section

; Function: main
main:
    ENTER 140
    PUSH R4
    PUSH R5
    PUSH R6
    PUSH R7
    PUSH R8
    PUSH R9
    PUSH R10
    PUSH R11
    MOV R0, .L..16
    PUSH R0
    POP R0
    CALL print_string
.L.begin.1:
    MOV R0, 1
    CMP R0, 0
    JMPE .L..17
    LEA R0, [BP - 1]
    MOV R1, 0
    MOV R2, R0
    MOV [R2], R1
    LEA R0, [BP - 1]
    PUSH R0
    CALL get_char
    CBW R0
    POP R1
    MOV [R1], R0
    MOV R0, 13
    PUSH R0
    LEA R0, [BP - 1]
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
    MOV R0, 10
    CBW R0
    PUSH R0
    POP R0
    CALL print_char
    JMP .L.end.2
.L.else.2:
    LEA R0, [BP - 1]
    MOV R0, [R0]
    CBW R0
    PUSH R0
    POP R0
    CALL print_char
.L.end.2:
.L..18:
    JMP .L.begin.1
.L..17:
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
    JMPE .L.true.5
    MOV R0, 0
    JMP .L.end.5
.L.true.5:
    MOV R0, 1
.L.end.5:
    CMP R0, 0
    JMPE .L.else.4
    MOV R0, 48
    CBW R0
    PUSH R0
    POP R0
    CALL print_char
    JMP .L.return.print_number
    JMP .L.end.4
.L.else.4:
.L.end.4:
    MOV R0, 0
    PUSH R0
    LEA R0, [BP - 30]
    MOV R0, [R0]
    POP R1
    CMP R0, R1
    JMPL .L.true.7
    MOV R0, 0
    JMP .L.end.7
.L.true.7:
    MOV R0, 1
.L.end.7:
    CMP R0, 0
    JMPE .L.else.6
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
    JMP .L.end.6
.L.else.6:
.L.end.6:
.L.begin.8:
    LEA R0, [BP - 30]
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
.L..11:
    JMP .L.begin.8
.L..10:
.L.begin.10:
    LEA R0, [BP - 10]
    MOV R0, [R0]
    PUSH R0
    MOV R0, 0
    POP R1
    CMP R0, R1
    JMPL .L.true.11
    MOV R0, 0
    JMP .L.end.11
.L.true.11:
    MOV R0, 1
.L.end.11:
    CMP R0, 0
    JMPE .L..12
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
.L..13:
    JMP .L.begin.10
.L..12:
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
.L.begin.12:
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
    JMPNE .L.true.13
    MOV R0, 0
    JMP .L.end.13
.L.true.13:
    MOV R0, 1
.L.end.13:
    CMP R0, 0
    JMPE .L..6
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
.L..7:
    JMP .L.begin.12
.L..6:
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

; Function: get_char
get_char:
    ENTER 138
    PUSH R4
    PUSH R5
    PUSH R6
    PUSH R7
    PUSH R8
    PUSH R9
    PUSH R10
    PUSH R11
    PUSH R0
    MOV R0, 0
    INT 0x11
    MOV R1, kb_result
    MOV [R1], R2
    POP R0
    MOV R0, kb_result
    MOV R0, [R0]
    CBW R0
    JMP .L.return.get_char
.L.return.get_char:
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