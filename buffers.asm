; Walks the buffer chain and gives prints out the state of the buffers.

[map all ./Listings/buffers.map]
[DEFAULT REL]
BITS 64
%include "./Include/dosMacro.mac"
%include "./Include/dosStruc.inc"
%include "./Include/dosError.inc"
freeDriveMarker equ "!"

    lea rdx, crlf
    mov eax, 0900h
    int 21h    
    
    mov eax, 5200h
    int 21h
    mov rbx, qword [rbx + sysVars.bufHeadPtr]   ;Get the start of the buffer head pointer
.buildBufferLine:
    ;Here we start building it properly
    mov al, byte [rbx + bufferHdr.driveNumber]
    cmp al, -1
    jne .notFree
    mov al, freeDriveMarker
    jmp short .putInLetter
.notFree:
    add al, "A" ;Convert from 0 based drive number to drive letter
.putInLetter:
    mov byte [bufferLine.driveLetter], al
    mov al, byte [rbx + bufferHdr.bufferFlags]  ;Get the flags
    test al, dosBuffer
    jz .notDosBuf
    mov ecx, "DOS "
    jmp short .putType
.notDosBuf:
    test al, fatBuffer
    jz .notFatBuffer
    mov ecx, "FAT "
    jmp short .putType
.notFatBuffer:
    test al, dirBuffer
    jz .notDirBuffer
    mov ecx, "DIR "
    jmp short .putType
.notDirBuffer:
    mov ecx, "DATA"
.putType:
    mov dword [bufferLine.bufferType], ecx  ;Store the string
    mov ecx, "F"
    test al, refBuffer  ;Is this a refBuffer
    jz .notRef
    mov ecx, "T"
.notRef:
    mov byte [bufferLine.bufferRefFlag], cl ;Store T/F for referenced flag
    mov ecx, "F"
    test al, dirtyBuffer
    jz .notDirty
    mov ecx, "T"
.notDirty:
    mov byte [bufferLine.bufferDirtFlag], cl    ;Store T/F for dirty flag
    lea rdx, bufferLine
    mov eax, 0900h  ;Write line to STDOUT
    int 21h

    mov rax, qword [rbx + bufferHdr.bufferLBA]  ;Get the LBA number
    push rbx    ;Save the ptr to the buffer on the stack
;Takes the qword in eax and prints its decimal representation
    xor ecx, ecx
    xor ebx, ebx    ;Store upper 8 nybbles here
    test eax, eax
    jnz .notZero
    mov ecx, "0"
    mov ebp, 1  ;Print one digit
    jmp short .dpfb2
.notZero:
    xor ebp, ebp  ;Use bp as #of digits counter
    mov esi, 0Ah  ;Divide by 10
.dpfb0:
    inc ebp
    cmp ebp, 8
    jb .dpfb00
    shl rbx, 8    ;Space for next nybble
    jmp short .dpfb01
.dpfb00:
    shl rcx, 8    ;Space for next nybble
.dpfb01:
    xor edx, edx
    div rsi
    add dl, '0'
    cmp dl, '9'
    jbe .dpfb1
    add dl, 'A'-'0'-10
.dpfb1:
    cmp ebp, 8
    jb .dpfb10
    mov bl, dl ;Add the bottom bits
    jmp short .dpfb11
.dpfb10:
    mov cl, dl    ;Save remainder byte
.dpfb11:
    test rax, rax
    jnz .dpfb0
.dpfb2:
    cmp ebp, 8
    jb .dpfb20
    mov dl, bl
    shr rbx, 8
    jmp short .dpfb21
.dpfb20:
    mov dl, cl    ;Get most sig digit into al
    shr rcx, 8    ;Get next digit down
.dpfb21:
    mov ah, 02h
    int 21h
    dec ebp
    jnz .dpfb2
    ;Here we are done, walk the buffer chain
    pop rbx

    lea rdx, crlf
    mov eax, 0900h
    int 21h    

    mov rbx, qword [rbx + bufferHdr.nextBufPtr]
    cmp rbx, -1
    jne .buildBufferLine
    mov eax, 4C00h
    int 21h

crlf:   db 0Ah,0Dh,"$"

bufferLine:
    db "Drive: "
.driveLetter:   ;Exclaimation mark means free
    db "  | Type: "
.bufferType:  
    db "    | Ref: "
.bufferRefFlag:
    db "  | Dirty: "
.bufferDirtFlag:
    db "  | Sector: $"
.bufferSectorNumber: 
bufferLineLen   equ $ - bufferLine
