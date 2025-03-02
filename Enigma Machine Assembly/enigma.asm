%include "../include/io.mac"

;; defining constants, you can use these as immediate values in your code
LETTERS_COUNT EQU 26
ASCII_A EQU 65

section .data
    extern len_plain
    i dw 0
    j dw 0
    rot dd 0
    offset dd 0
    dir dd 0
    ok db 0

    text_size dw 0
    current_letter dw 0
    encrypt_done dw 0
    
section .bss
    conf1 resb 26

section .text
    global rotate_x_positions
    global enigma

shift_left:
    mov edi, dword [offset]
    mov word [i], di
    cmp edi, 0 ; Daca offset e 0, se iese
    je after_if

    left_for_i:
        mov eax, dword [rot]
        mov ebx, LETTERS_COUNT
        imul ebx, eax
        mov eax, 0
        mov al, byte [ecx + ebx]
        push eax

        left_for_j:
            mov edi, 0
            mov di, word [j]
            mov esi, edi
            add esi, ebx
            add esi, 1
            mov edx, 0
            mov dl, byte [ecx + esi]
            sub esi, 1
            mov byte [ecx + esi], dl

            mov edi, 0
            add word [j], 1
            mov di, word [j]
            cmp edi, 25
        jl left_for_j

        pop eax
        mov esi, 25
        add esi, ebx 
        mov byte [ecx + esi], al
        mov word [j], 0
        sub word [i], 1
        mov edi, 0
        mov di, word [i]
        cmp edi, 0
    jg left_for_i

jmp after_if

shift_right:
    mov edi, dword [offset]
    mov word [i], di
    cmp edi, 0 ; Daca offset e 0, se iese
    je after_if
    mov word [j], 25

    right_for_i:
        mov eax, dword [rot]
        mov ebx, LETTERS_COUNT
        imul ebx, eax
        mov esi, 25
        add esi, ebx
        mov eax, 0
        mov al, byte [ecx + esi]
        push eax

        right_for_j:
            mov edi, 0
            mov di, word [j]
            mov esi, edi
            add esi, ebx
            sub esi, 1
            mov edx, 0
            mov dl, byte [ecx + esi]
            add esi, 1
            mov byte [ecx + esi], dl

            mov edi, 0
            sub word [j], 1
            mov di, word [j]
            cmp edi, 0
        jg right_for_j

        pop eax
        mov byte [ecx + ebx], al
        mov word [j], 25
        sub word [i], 1
        mov edi, 0
        mov di, word [i]
        cmp edi, 0
    jg right_for_i

jmp after_if

; void rotate_x_positions(int x, int rotor, char config[10][26], int forward);
rotate_x_positions:
    ;; DO NOT MODIFY
    push ebp
    mov ebp, esp
    pusha

    mov eax, [ebp + 8]  ; x
    mov ebx, [ebp + 12] ; rotor
    mov ecx, [ebp + 16] ; config (address of first element in matrix)
    mov edx, [ebp + 20] ; forward
    ;; DO NOT MODIFY
    ;; TODO: Implement rotate_x_positions
    ;; FREESTYLE STARTS HERE
    push edx
    push eax
    push ebx
    mov dword [dir], edx
    mov dword [offset], eax
    mov eax, 0
    mov al, byte [ecx]
    
    mov eax, 2
    imul ebx, eax
    mov dword [rot], ebx
    mov byte [ok], 0
    shift:
        mov word [i], 0
        mov word [j], 0
        mov edx, dword [dir]
        cmp edx, 0
        je shift_left ; Se shifteaza la stanga

        mov edx, dword [dir]
        cmp edx, 1
        je shift_right ; Se shifteaza ;a dreapta
        after_if:

        add dword [rot], 1
        add byte [ok], 1
        mov eax, 0
        mov al, byte [ok]
        cmp eax, 1
    jle shift

    pop ebx
    pop eax
    pop edx

    ;; FREESTYLE ENDS HERE
    ;; DO NOT MODIFY
    popa
    leave
    ret
    ;; DO NOT MODIFY

rotor3_ready:
    mov eax, 0
    mov edx, 0
    mov al, byte [ebx + 2]
    mov dl, byte [ecx + 2]
    cmp eax, edx
    je rotor2_ready

    mov al, byte [ebx + 1]
    mov dl, byte [ecx + 1]
    cmp eax, edx
    je rotor2_ready
    return_to_rotor3:

    ; functia rotate
    push 0
    push esi
    push 2
    push 1

    call rotate_x_positions
    add esp, 16
    ;

    add byte [ebx + 2], 1

    sub byte [ebx + 2], 65
    push ecx
    push eax
    push edx

    ; Shiftarea literei din key
    ; Asemanator cu task 1
    mov edx, 0
    mov eax, 0
    mov al, byte [ebx + 2]
    mov ecx, LETTERS_COUNT
    div ecx
    mov byte [ebx + 2], dl
    add byte [ebx + 2], ASCII_A
    ;

    pop edx
    pop eax
    pop ecx
jmp after_permutation

rotor2_ready:
    mov eax, 0
    mov edx, 0
    mov al, byte [ebx + 1]
    mov dl, byte [ecx + 1]
    cmp eax, edx
    je rotor1_ready
    return_to_rotor2:

    ; functia rotate
    push 0
    push esi
    push 1
    push 1

    call rotate_x_positions
    add esp, 16
    ;

    add byte [ebx + 1], 1

    sub byte [ebx + 1], ASCII_A
    push ecx
    push eax
    push edx

    ; Shiftarea literei din key
    ; Asemanator cu task 1
    mov edx, 0
    mov eax, 0
    mov al, byte [ebx + 1]
    mov ecx, LETTERS_COUNT
    div ecx
    mov byte [ebx + 1], dl
    add byte [ebx + 1], ASCII_A
    ;

    pop edx
    pop eax
    pop ecx
jmp return_to_rotor3
    
rotor1_ready:
    ; functia rotate
    push 0
    push esi
    push 0
    push 1

    call rotate_x_positions
    add esp, 16
    ;
    
    add byte [ebx + 0], 1

    sub byte [ebx + 0], ASCII_A
    push ecx
    push eax
    push edx

    ; Shiftarea literei din key
    ; Asemanator cu task 1
    mov edx, 0
    mov eax, 0
    mov al, byte [ebx + 0]
    mov ecx, LETTERS_COUNT
    div ecx
    mov byte [ebx + 0], dl
    add byte [ebx + 0], ASCII_A
    ;

    pop edx
    pop eax
    pop ecx
jmp return_to_rotor2

verify_encrypt_exit:
    add word [encrypt_done], 1 ; Flag de oprire al labelului encrypt
jmp exit_encrypt

traverse:
    push ebp
    mov ebp, esp
    pusha

    mov ecx, [ebp + 8]
    mov edi, [ebp + 12]

    mov eax, 0
    mov ax, word [i]
    mov edx, 0
    imul edi, LETTERS_COUNT
    add edi, eax
    mov dl, byte [esi + edi]
    imul ecx, LETTERS_COUNT
    mov word [i], -1
    search_position:
        add word [i], 1
        mov eax, 0
        mov ax, word [i]
        mov ebx, 0
        add ecx, eax
        mov bl, byte [esi + ecx]
        sub ecx, eax
        cmp ebx, edx
    jne search_position

    popa
    leave
    ret
; void enigma(char *plain, char key[3], char notches[3], char config[10][26], char *enc);
enigma:
    ;; DO NOT MODIFY
    push ebp
    mov ebp, esp
    pusha

    mov eax, [ebp + 8]  ; plain (address of first element in string)
    mov ebx, [ebp + 12] ; key
    mov ecx, [ebp + 16] ; notches
    mov edx, [ebp + 20] ; config (address of first element in matrix)
    mov edi, [ebp + 24] ; enc
    ;; DO NOT MODIFY
    ;; TODO: Implement enigma
    ;; FREESTYLE STARTS HERE

    mov word [encrypt_done], 0
    mov esi, [len_plain]
    mov word [text_size], si
    mov word [current_letter], 0
    
    ; Se schimba config din edx in esi
    ; edx e mult mai util pentru prelucrare decat esi
    push edx
    pop esi

    encrypt:
        push eax
        jmp rotor3_ready
        after_permutation:
        pop eax

        push eax
        push ebx
        push ecx
        push edi

        mov edx, 0
        mov dx, word [current_letter]
        mov dl, byte [eax + edx]
        mov eax, 0 
        mov al, dl
        ; START, eax

        ; Fiecare label reprezinta un for care gaseste pozitia ce contine
        ; litera ce trebuie gasita

        ; de aici pana la linia 450 reprezinta traseul literei
        ; ce trebuie criptata

        sub eax, ASCII_A
        mov edx, 0
        mov dl, byte [esi + LETTERS_COUNT * 8 + eax]
        ; PLUGBOARD R, edx

        mov word [i], -1
        inside_plugboard:

            add word [i], 1
            mov eax, 0
            mov ax, word [i]
            mov ebx, 0
            mov bl, byte [esi + LETTERS_COUNT * 9 + eax]
            cmp ebx, edx
        jne inside_plugboard
        ; PLUGBOARD L, ebx

        push 5 ; ROTOR 3
        push 4
        call traverse
        add esp, 8

        push 3 ; ROTOR 2
        push 2
        call traverse
        add esp, 8

        push 1 ; ROTOR 1
        push 0
        call traverse
        add esp, 8

        push 7 ; REFLECTOR
        push 6
        call traverse
        add esp, 8

        push 0 ; ROTOR 1
        push 1
        call traverse
        add esp, 8

        push 2 ; ROTOR 2
        push 3
        call traverse
        add esp, 8

        push 4 ; ROTOR 3
        push 5
        call traverse
        add esp, 8

        mov eax, 0
        mov ax, word [i]
        mov edx, 0
        mov dl, byte [esi + LETTERS_COUNT * 9 + eax]
        ; PLUGBOARD L, edx

        mov word [i], -1
        outside_plugboard:
            add word [i], 1
            mov eax, 0
            mov ax, word [i]
            mov ebx, 0
            mov bl, byte [esi + LETTERS_COUNT * 8 + eax]
            cmp ebx, edx
        jne outside_plugboard
        ; PLUGBOARD R, ebx

        mov edx, 0
        mov dx, word [current_letter]
        pop edi
        mov byte [edi + edx], bl
        ; RESULT, [edi + edx]
        pop ecx
        pop ebx
        pop eax

        push eax

        add word [current_letter], 1
        mov edx, 0
        mov dx, word [current_letter]
        mov eax, 0
        mov ax, word [text_size]
        cmp edx, eax
        je verify_encrypt_exit
        exit_encrypt:

        pop eax


        mov edx, 0
        mov dx, word [encrypt_done]
        cmp edx, 0
    je encrypt

    ;; FREESTYLE ENDS HERE
    ;; DO NOT MODIFY
    popa
    leave
    ret
    ;; DO NOT MODIFY