echo "hello .gdbinit !\n"
# see https://devarea.com/10-things-you-can-only-do-with-gdb/#.XiPwDMj0n8A\n


set prompt \033[1;92m[gdb] > \033[0m
#-------------------------------------------------------------------- Scripts ---
# set data-directory ./scripts
# source ./scripts/.gdbinit-gef.py

#--------------------------------------------------------Better GDB defaults ---
set history save
set verbose off
set print pretty on
set print array off
set print array-indexes on

define hook-backtrace
echo \033[01;33m
end

define hookpost-backtrace
echo \033[0m\n
end

# Don't wrap line or the coloring regexp won't work.
set width 0

#------------------------------------------------------------------- Commands ---
define sf
	where
	info args
	info locals
	end



#---------------------------------------------------------------- Playground ---
# set args "toolongstring"
break builtin_list
# commands
# 	explore a
# 	print b
# 	print *argv@2
# 	end
run

explore arguments->cells[0]->cells[0]