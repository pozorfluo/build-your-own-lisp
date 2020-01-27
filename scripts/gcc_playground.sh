# generate asm
# compile with
#   gcc -S -o canary_no-ssp.s -fno-stack-protector canary.c
#   gcc -S -o canary_ssp.s -fstack-protector canary.c

# compare generated assembly code

# debug
#   gdb -q ../../bin/canary
#   list
#   break 13
#   run 'argsdjflsdlfkljklsdjfksdlfj'
#   step
gcc -S -o error_message.s error_message.c
gcc -S -o lisp_gotos.s lisp.c