start:
    CLI
    MOV SP, 0x7C00
    MOV BP, SP
    CALL setup_ivt
    STI
    
    MOV R1, 0x1F        ; White on Blue
    CALL clear_screen
    
    LEA R0, bios_msg
    CALL print_string
    
    MOV R0, 20
    CALL delay
    
    LEA R0, cpu_test_msg
    CALL print_string
    CALL cpu_test
    LEA R0, ok_msg
    CALL print_string

    MOV R0, 10
    CALL delay

    LEA R0, mem_test_msg
    CALL print_string
    CALL memory_test
    LEA R0, ok_msg
    CALL print_string
    
    MOV R0, 10
    CALL delay

    LEA R0, boot_msg
    CALL print_string
    
    MOV R0, 50
    CALL delay
    
    JMP 0x1000

clear_screen:
    PUSHA
    MOV R12, R1
    SHL R12, 8
    OR R12, 32         ; Space character with attribute in R1
    MOV R0, 0x07       ; Port 0x07: Screen Fill
    OUT R0, R12
    MOV R10, 0
    CALL set_cursor_pos
    POPA
    RET

print_string:
    PUSHA
print_str_loop:
    MOV R2, [R0]
    AND R2, 0xFF
    CMP R2, 0
    JMPE print_str_done
    MOV R3, R2
    CALL print_char
    INC R0
    JMP print_str_loop
print_str_done:
    POPA
    RET

print_char:
    PUSHA
    LEA R13, current_attribute
    MOV R1, [R13]
    
    CMP R3, 10
    JMPE handle_newline
    
    CALL get_cursor_pos
    MOV R10, R4
    
    MOV R2, R1
    SHL R2, 8
    OR R2, R3
    
    MOV R0, 0x05       ; Port 0x05: VRAM Address
    OUT R0, R10
    MOV R0, 0x06       ; Port 0x06: VRAM Data
    OUT R0, R2
    
    INC R10
    CALL set_cursor_pos
    JMP print_char_end

handle_newline:
    CALL get_cursor_pos
    MOV R10, R4
    MOV R11, 80
    DIV R10, R11
    INC R10
    MUL R10, R11
    CALL set_cursor_pos

print_char_end:
    POPA
    RET

get_cursor_pos:
    PUSH R0
    PUSH R1
    PUSH R2
    PUSH R3
    MOV R0, 0x01       ; Port 0x01: CRTC Address
    MOV R1, 0x0E       ; High byte
    OUT R0, R1
    MOV R0, 0x02       ; Port 0x02: CRTC Data
    IN R2, R0
    MOV R0, 0x01
    MOV R1, 0x0F       ; Low byte
    OUT R0, R1
    MOV R0, 0x02
    IN R3, R0
    SHL R2, 8
    OR R2, R3
    MOV R4, R2
    POP R3
    POP R2
    POP R1
    POP R0
    RET

set_cursor_pos:
    PUSHA
    MOV R2, R10
    MOV R3, R2
    SHR R2, 8
    MOV R0, 0x01       ; Port 0x01: CRTC Address
    MOV R1, 0x0E       ; High Byte
    OUT R0, R1
    MOV R0, 0x02       ; Port 0x02: CRTC Data
    OUT R0, R2
    AND R3, 0xFF
    MOV R0, 0x01
    MOV R1, 0x0F       ; Low Byte
    OUT R0, R1
    MOV R0, 0x02
    OUT R0, R3
    POPA
    RET

; INT 0x00-0x09 reserved for hardware interrupts
setup_ivt:
    LEA R0, div_zero_handler
    MOV R1, 0x00
    MOV [R1], R0
    LEA R0, invalid_op_handler
    MOV R1, 0x01
    MOV [R1], R0
    LEA R0, video_service
    MOV R1, 0x10
    MOV [R1], R0
    LEA R0, keyboard_service
    MOV R1, 0x11
    MOV [R1], R0
    RET

video_service:
    PUSHA
    CMP R0, 0x01
    JMPE vs_print_char
    CMP R0, 0x02
    JMPE vs_set_cursor
    CMP R0, 0x03
    JMPE vs_get_cursor
    CMP R0, 0x04
    JMPE vs_clear_screen
    CMP R0, 0x05
    JMPE vs_cursor_enable
    JMP video_service_end
vs_print_char:
    MOV R3, R2
    CALL print_char
    JMP video_service_end
vs_get_cursor:
    CALL get_cursor_pos
    MOV R2, R4
    JMP video_service_end
vs_set_cursor:
    MOV R10, R2
    CALL set_cursor_pos
    JMP video_service_end
vs_clear_screen:
    LEA R13, current_attribute
    MOV [R13], R2
    MOV R1, R2
    CALL clear_screen
    JMP video_service_end
vs_cursor_enable:
    MOV R0, 0x04
    OUT R0, R2
    JMP video_service_end
video_service_end:
    POPA
    IRET
	
keyboard_service:
    CMP R0, 0x00
    JMPE kb_getc
    CMP R0, 0x01
    JMPE kb_status
    IRET

kb_getc:
    IN R1, 0x08
    CMP R1, 0
    JMPE kb_getc
    IN R2, 0x09
    IRET

kb_status:
    IN R2, 0x08
    IRET

cpu_test:
    PUSHA
    MOV R0, 0xAAAA
    MOV R1, 0x5555
    ADD R0, R1
    CMP R0, 0xFFFF
    JMPNE cpu_test_fail
    MOV R0, 0x1000
    MOV R1, 0x0100
    SUB R0, R1
    CMP R0, 0x0F00
    JMPNE cpu_test_fail
    POPA
    RET
cpu_test_fail:
    LEA R0, cpu_fail_msg
    CALL print_string
    cpu_loop: JMP cpu_loop

memory_test:
    PUSHA
    MOV R10, MEM_TEST_START
mem_test_loop:
    CMP R10, MEM_TEST_END
    JMPGE mem_test_success
    MOV R0, 0xAAAA      
    MOV [R10], R0
    MOV R1, [R10]
    CMP R0, R1
    JMPNE memory_test_fail
    INC R10
    JMP mem_test_loop
mem_test_success:
    POPA
    RET

memory_test_fail:
    LEA R0, mem_fail_msg
    CALL print_string
    mem_loop: JMP mem_loop

div_zero_handler:
    LEA R0, div_zero_msg
    CALL print_string
    fault_loop_1: JMP fault_loop_1
invalid_op_handler:
    LEA R0, invalid_op_msg
    CALL print_string
    fault_loop_2: JMP fault_loop_2

delay:
    PUSHA
delay_outer:
    CMP R0, 0
    JMPE delay_done
delay_inner:
    IN R1, 0x03        ; Port 0x03: Timer
    CMP R1, 1
    JMPNE delay_inner
    DEC R0
    JMP delay_outer
delay_done:
    POPA
    RET

bios_msg:               DB "CPU16 BIOS", 10, 0
cpu_test_msg:           DB "Testing CPU... ", 0
mem_test_msg:           DB "Testing Memory... ", 0
ok_msg:                 DB "Passed.", 10, 0
cpu_fail_msg:           DB 10, "CPU Test Failed!", 0
mem_fail_msg:           DB 10, "Memory Test Failed!", 0
div_zero_msg:           DB 10, "Div by Zero!", 0
invalid_op_msg:         DB 10, "Invalid Opcode!", 0
boot_msg:               DB 10, "Booting Program...", 0
current_attribute:      DW 0x1F
number_buffer:          DB 32, 32, 32, 32, 32, 32, 0
number_buffer_end:
MEM_TEST_START:         DW 0x2000
MEM_TEST_END:           DW 0x4000