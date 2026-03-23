    JMP main
    ; Data Section
.L..13:
    DB 55
    DB 45
    DB 49
    DB 48
    DB 32
    DB 105
    DB 115
    DB 32
    DB 0
.L..12:
    DB 10
    DB 0
.L..11:
    DB 50
    DB 43
    DB 55
    DB 32
    DB 105
    DB 115
    DB 32
    DB 0
.L..10:
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
.L..9:
    DB 109
    DB 97
    DB 105
    DB 110
    DB 0
.L..8:
    DB 109
    DB 97
    DB 105
    DB 110
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
.L..2:
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
    MOV R0, .L..10
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
    MOV R0, .L..11
    PUSH R0
    POP R0
    CALL print_string
    LEA R0, [BP - 8]
    MOV R0, [R0]
    PUSH R0
    POP R0
    CALL print_number
    MOV R0, .L..12
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
    MOV R0, .L..13
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
    JMPE .L..14
.L..15:
    JMP .L.begin.1
.L..14:
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
.L.begin.2:
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
    JMPNE .L.true.3
    MOV R0, 0
    JMP .L.end.3
.L.true.3:
    MOV R0, 1
.L.end.3:
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
    JMP .L.begin.2
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

; Function: print_number
print_number:
    ENTER 4
    PUSH R4
    PUSH R5
    PUSH R6
    PUSH R7
    PUSH R8
    PUSH R9
    PUSH R10
    PUSH R11
    MOV [BP - 4], R0
    MOV R0, num_to_print
    PUSH R0
    LEA R0, [BP - 4]
    MOV R0, [R0]
    POP R1
    MOV [R1], R0
    PUSH R0
    PUSH R2
    MOV R0, 5
    MOV R2, num_to_print
    MOV R2, [R2]
    INT 32
    POP R2
    POP R0
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
    INT 32
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