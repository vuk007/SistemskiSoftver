.global start
.extern func
.section text
start:
 call func
 halt
.end
