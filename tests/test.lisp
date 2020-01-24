;;.lisp extension used to get some syntax highlighting

;; chapter 9 : S-Expression
	+ 1 (* 7 5) 3
	;; 39
	(- 100)
	;; -100
	
	;; ()
	/
	;; /
	(/ ())
	;; Error: Cannot operate on non-number!

;; chapter 10 : Q-Expression
	list 1 2 3 4
	;; {1 2 3 4}
	{head (list 1 2 3 4)}
	;; {head (list 1 2 3 4)}
	eval {head (list 1 2 3 4)}
	;; {1}
	tail {tail tail tail}
	;; {tail tail}
	eval (tail {tail tail {5 6 7}})
	;; {6 7}
	eval (head {(+ 1 2) (+ 10 20)})
	;; 3
	eval {}
	;; ()
	head {(+ 1 2) (+ 10 20)}
    ;; {(+ 1.000000 2.000000 ) }
	join (head {(+ 1 2) (+ 10 20)})
	;; {(+ 1.000000 2.000000 ) }
	join (head {(+ 1 2) (+ 10 20)}) {5 6 7}
	;; {(+ 1.000000 2.000000 ) 5.000000 6.000000 7.000000 }
	eval {(+ 5 3 (eval (head{5 3 5})))}
	;; 13.000000
	join {5} (head {5 3 5}) 
	;; {5.000000 5.000000 }
	join (head{- + /}) (head{5 3 5}) (tail{7 5 7})
	;; {- 5.000000 5.000000 7.000000 }
	eval (join (head{- + /}) (head{5 3 5}) (tail{7 5 7}))
    ;; -7.000000

;; kcal
	eval {+ 6000 1554 1939 1529 1474}
	/ (+ 6000 1554 1939 1529 1474) 6
	* 2300 6
	/ (- (* 2300 6) 6000) 5
	- (- (* 2300 6) 6000) (+ 1554 1939 1529 1474)
	+ 1554 1939 1529 1474 (- (- (* 2300 6) 6000) (+ 1554 1939 1529 1474))
	/ (+ 1554 1939 1529 1474 (- (- (* 2300 6) 6000) (+ 1554 1939 1529 1474)) 6000) 6

    - (- (* 2300 6) 6000) (+ 1554 1939 1529 2140 356)

;; bonus mark : cons
;; valgrind --track-origins=yes ./bin/lisp
	cons 1
	cons 1 {4 5 5} 5
	cons 1 {4 5 5} 5 67 {4 6 7}
	cons {455 5 } 1
	;; passing
	list
	list 1
	list 1 {4 5 5}
	cons 1 {4 5 5}
	cons (+ 1 5.55) {4e44 5 5}
	cons (1) {4 5 5}
	cons {1} {4 5 5}
	cons 5 (list 1 { 45 54 54 })

;; bonus mark : len
	len 0 4
	len (255)
	len 1
	len {54 4} {54 4}
	;; passing
	len 
	len {54 4}
	len ({54 4})
	len {}
	len {1}
	len {1 (+ 45  54 ) {5454 54 56e64 -534.44e-7 }}
	+ 4 (len {1 (+ 45  54 ) {5454 54 56e64 -534.44e-7 }})
	+ 4 (len {1 (+ 45  54 ) {5454 54 56e64 -534.44e-7 }}) (eval{len { 7 4 5}})
	len (cons 5 (list 1 { 45 54 54 }))

;; bonus mark : init
;; valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./bin/lisp
	init 0 1 4
	init 0
	init (+ 0 1 4)
	init {} (+ 0 1 4)
	init {5 5} (* 0 1 4)
	init {}
	init {5 5} {0 1 4}
	;; passing
	init
	init {5 5}
	init {5}
	init {5 (- 5 4 3 5)}
	init {5 (- 5 4 3 5) 5}
	init {{5 7 52.5e5} (- 5 4 3 5) 5}
	init {(eval{len {* 7 4 5}}) {5 7 52.5e5} {(- 5 4 3 5) 5}}
	+ 4 (len {1 (+ 45  54 ) {5454 54 56e64 -534.44e-7 }}) (eval{len { 7 4 5}}) 
	init {{5 7 52.5e5} {(- 5 4 3 5) 5} (eval{len { 7 4 5}})}
	eval {+ (len {eval(init{* 7 4 5})}) 5}

;; chapter 11 : variables
	+
	;; <function>
	eval (head {5 10 11 15})
	;; 5
	eval (head {+ - + - * /})
	;; <function>
	(eval (head {+ - + - * /})) 10 20
	;; 30
	hello
	;; Error: unbound symbol!
	add 3 4
	+ 3 4
	sub 5 2
	- 5 2
	mul 4 6
	* 4 6
	div 6 7
	/ 6 7
	div 6 0
	/ 6 0
	mod 6.35 4.21
	% 6.35 4.21
	pow 9 0
	^ 9 0
	pow 9 -4
	^ 9 -4
	max 546 5
	> 546 5
	min 546 5
	< 546 5
	head {+ 5 654 542 2.212 6.3e4}
	tail {+ 5 654 542 2.212 6.3e4}
	list 5 654 542 2.212 6.3e4
	init {+ 5 654 542 2.212 6.3e4}
	eval {+ 5 654 542 2.212 6.3e4}
	join {+ 5 654 542 2.212 6.3e4} { 5 654 542 2.212 6.3e4}
	cons {+ 5 654 542 2.212 6.3e4} { 5 654 542 2.212 6.3e4}
	len  {+ 5 654 542 2.212 6.3e4}