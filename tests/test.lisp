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
	list 1
	list 1 {4 5 5}
	cons 1
	cons 1 {4 5 5} 5
	cons 1 {4 5 5} 5 67 {4 6 7}
	cons {455 5 } 1
	cons 1 {4 5 5}
	cons (+ 1 5.55) {4e44 5 5}
	cons (1) {4 5 5}
	cons {1} {4 5 5}

;; bonus mark : len
	len 
	len 0 4
	len 1
	len (255)
	len {54 4} {54 4}
	len {54 4}
	len ({54 4})
	len {}
	len {1}
	len {1 (+ 45  54 ) {5454 54 56e64 -534.44e-7 }}
	+ 4 (len {1 (+ 45  54 ) {5454 54 56e64 -534.44e-7 }})
	+ 4 (len {1 (+ 45  54 ) {5454 54 56e64 -534.44e-7 }}) (eval{len { 7 4 5}})