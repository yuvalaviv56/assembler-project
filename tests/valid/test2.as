MAIN: mov r3, r7
LOOP: add #5, r1
      jmp END
      cmp r2, r4
END: stop
NUM: .data 10, -5, 100
STR: .string "hello"
.entry MAIN
.entry LOOP
.extern EXTERNAL_VAR