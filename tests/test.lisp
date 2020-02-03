join (head {(+ 1 2) (+ 10 20)}) {5 6 7}
;; {(+ 1.000000 2.000000 ) 5.000000 6.000000 7.000000 }
list 1 2 3 4
;;{${fg_yellow}1${reset} ${fg_yellow}2${reset} ${fg_yellow}3${reset} ${fg_yellow}4${reset}}

	Test("+ 5 5",
		"10.00000"),
	Test("+ 1 (* 7 5) 3",
		"39.000000"),
	Test("(- 100)",
		"-100.000000"),
	Test("/",
		"<div>"),
	Test("(/ ())",
		"Error :"),
	Test("",
		"()"),
	Test("list 1 2 3 4",
		f"{{{fg_yellow}1.000000{reset} {fg_yellow}2.000000{reset} {fg_yellow}3.000000{reset} {fg_yellow}4.000000{reset} }}"),
	# Test("{head (list 1 2 3 4)}",
	# 	f"{head (list 1.000000 2.000000 3.000000 4.000000 ) }"),
	Test("eval {head (list 1 2 3 4)}",
		f"{{{fg_yellow}1.000000{reset} }}"),
	Test("tail {tail tail tail}",
		f"{{{fg_cyan}tail{reset} {fg_cyan}tail{reset} }}"),
	Test("eval (tail {tail tail {5 6 7}})",
		f"{{{fg_yellow}6.000000{reset} {fg_yellow}7.000000{reset} }}"),
	Test("eval (head {(+ 1 2) (+ 10 20)})",
		"3.000000"),
	Test("tail {qsd kkl aio}",
		f"{{{fg_cyan}kkl{reset} {fg_cyan}aio{reset} }}")