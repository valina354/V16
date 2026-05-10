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
    
    JMP 0x1000

clear_screen:
    PUSHA
    MOV R12, R1
    SHL R12, 8
    OR R12, 32         ; Space character
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
    
    CMP R3, 8
    JMPE handle_backspace
    CMP R3, 10
    JMPE handle_newline
    
    CALL get_cursor_pos
    MOV R10, R4
    
    ; Check if we need to scroll before printing
    CALL check_scroll
    CALL get_cursor_pos
    MOV R10, R4

    MOV R2, R1
    SHL R2, 8
    OR R2, R3
    
    MOV R0, 0x05       ; VRAM Address
    OUT R0, R10
    MOV R0, 0x06       ; VRAM Data
    OUT R0, R2
    
    INC R10
    CALL set_cursor_pos
    JMP print_char_end
	
handle_backspace:
    CALL get_cursor_pos
    MOV R10, R4
    CMP R10, 0
    JMPE print_char_end
    DEC R10
    MOV R2, R1
    SHL R2, 8
    OR R2, 32
    MOV R0, 0x05
    OUT R0, R10
    MOV R0, 0x06
    OUT R0, R2
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
    CALL check_scroll       ; Scroll if newline pushed us off screen

print_char_end:
    POPA
    RET

check_scroll:
    PUSHA
    CALL get_cursor_pos
    CMP R4, 2000
    JMPL check_scroll_done

    ; Scroll Logic: Copy rows 1-24 to 0-23
    MOV R10, 0          ; Destination Index
scroll_loop:
    CMP R10, 1920       ; 80 * 24
    JMPGE scroll_clear_last_line
    
    MOV R11, R10
    ADD R11, 80         ; Source Index (Current + 80)
    
    MOV R0, 0x05
    OUT R0, R11         ; Set source pointer
    MOV R0, 0x06
    IN R12, R0          ; Read char+attr from source
    
    MOV R0, 0x05
    OUT R0, R10         ; Set destination pointer
    MOV R0, 0x06
    OUT R0, R12         ; Write char+attr to destination
    
    INC R10
    JMP scroll_loop

scroll_clear_last_line:
    MOV R10, 1920
    LEA R13, current_attribute
    MOV R12, [R13]
    SHL R12, 8
    OR R12, 32          ; Space with attribute
clear_last_loop:
    CMP R10, 2000
    JMPGE scroll_finish
    MOV R0, 0x05
    OUT R0, R10
    MOV R0, 0x06
    OUT R0, R12
    INC R10
    JMP clear_last_loop

scroll_finish:
    MOV R10, 1920
    CALL set_cursor_pos

check_scroll_done:
    POPA
    RET

get_cursor_pos:
    PUSH R0
    PUSH R1
    PUSH R2
    PUSH R3
    MOV R0, 0x01
    MOV R1, 0x0E
    OUT R0, R1
    MOV R0, 0x02
    IN R2, R0
    MOV R0, 0x01
    MOV R1, 0x0F
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
    MOV R0, 0x01
    MOV R1, 0x0E
    OUT R0, R1
    MOV R0, 0x02
    OUT R0, R2
    AND R3, 0xFF
    MOV R0, 0x01
    MOV R1, 0x0F
    OUT R0, R1
    MOV R0, 0x02
    OUT R0, R3
    POPA
    RET

setup_ivt:
    LEA R0, div_zero_handler
    MOV R1, 0x00
    MOV [R1], R0
    LEA R0, invalid_op_handler
    MOV R1, 0x01
    MOV [R1], R0
	LEA R0, gpf_handler
    MOV R1, 0x02
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
    CMP R0, 0x04
    JMPE vs_clear_screen
    JMP video_service_end
vs_print_char:
    MOV R3, R2
    CALL print_char
    JMP video_service_end
vs_clear_screen:
    LEA R13, current_attribute
    MOV [R13], R2
    MOV R1, R2
    CALL clear_screen
video_service_end:
    POPA
    IRET
	
keyboard_service:
    CMP R0, 0x00
    JMPE kb_getc
    IRET
kb_getc:
    IN R1, 0x08
    CMP R1, 0
    JMPE kb_getc
    IN R2, 0x09
    IRET

div_zero_handler:
    LEA R0, div_zero_msg
    CALL print_string
    f1: JMP f1
invalid_op_handler:
    LEA R0, invalid_op_msg
    CALL print_string
    f2: JMP f2
gpf_handler:
    LEA R0, gpf_msg
    CALL print_string
    gpf_stop: JMP gpf_stop 

bios_msg:               DB "CPU16 BIOS v1.1 - SCROLL ENABLED", 10, 0
div_zero_msg:           DB 10, "Div by Zero!", 0
invalid_op_msg:         DB 10, "Invalid Opcode!", 0
gpf_msg:                DB 10, "General Protection Fault!", 0
current_attribute:      DW 0x1F