# check gcc define's
# see https://stackoverflow.com/questions/142508/how-do-i-check-os-with-a-preprocessor-directive
# see https://askubuntu.com/questions/420981/how-do-i-save-terminal-output-to-a-file
gcc -dM -E - </dev/null > ./.tmp/gcc_defines.txt

gcc -dumpmachine

gcc -v