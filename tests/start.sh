ASSEMBLER=./src/asembler
LINKER=./src/linker
EMULATOR=emulator

${ASSEMBLER} -o main.o ./tests/main.s
${ASSEMBLER} -o handler.o ./tests/handler.s
${ASSEMBLER} -o isr_terminal.o ./tests/isr_terminal.s
${ASSEMBLER} -o isr_timer.o ./tests/isr_timer.s
${LINKER} -hex \
  -place=my_code@0x40000000 \
  -place=isr@0x55000000\
  -o program.hex \
  main.o isr_terminal.o isr_timer.o handler.o