# file: main.s

.section my_code
.extern hand
my_start:
    ld k, %r1
    ld hand, %r2
    ld $0xEFFFFFFE, %sp
    csrwr %r1, %handler
    ld $1, %r1
    push %r1
    #push %r1
    call 0xF0000000
    halt
.section my_code2
k:
.word 10
.section my_data
.word 105
.end
