MAIN: mov r3, r7
LOOP: add #5, r1
      jmp END
END: stop
NUM: .data 10
.entry MAIN
