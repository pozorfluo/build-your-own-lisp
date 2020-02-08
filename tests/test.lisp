env
<env>
env {}
;; {add sub mul div mod pow max min + - * / % ^ > < head tail list init eval join cons len def env }
env 0
;; {add sub mul div mod pow max min + - * / % ^ > < head tail list init eval join cons len def env }
def {hola} {5}
;; ()
env 0
;; {add sub mul div mod pow max min + - * / % ^ > < head tail list init eval join cons len def env hola }
head
;; <head>
join (head {(+ 1 2) (+ 10 20)}) {5 6 7}
;; {(+ 1.000000 2.000000 ) 5.000000 6.000000 7.000000 }
list 1 2 3 4
;; {1.000000 2.000000 3.000000 4.000000 }
tail (+ 5 5)
;; Error :  Function 'tail' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
tail {+ 5 5}
;; {5.000000 5.000000 }
tail head {+ 5 5}
;; Error :  Function 'tail' passed incorrect number of arguments !
;;          expected 1, got 2.
tail (head {+ 5 5})
;; {}
tail (len {+ 5 5})
;; Error :  Function 'tail' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
tail (len {+ 5 5})
;; Error :  Function 'tail' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
list 1 2 3 4
;; {1.000000 2.000000 3.000000 4.000000 }
+ 1 (* 7 5) 3
;; 39.000000
(- 100)
;; -100.000000

;; ()
/
;; <div>
(/ ())
;; Error :  Cannot operate on non-number !
list 1 2 3 4
;; {1.000000 2.000000 3.000000 4.000000 }
{head (list 1 2 3 4)}
;; {head (list 1.000000 2.000000 3.000000 4.000000 ) }
eval {head (list 1 2 3 4)}
;; {1.000000 }
tail {tail tail tail}
;; {tail tail }
eval (tail {tail tail {5 6 7}})
;; {6.000000 7.000000 }
eval (head {(+ 1 2) (+ 10 20)})
;; 3.000000
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
eval {+ 6000 1554 1939 1529 1474}
;; 12496.000000
/ (+ 6000 1554 1939 1529 1474) 6
;; 2082.666667
* 2300 6
;; 13800.000000
/ (- (* 2300 6) 6000) 5
;; 1560.000000
- (- (* 2300 6) 6000) (+ 1554 1939 1529 1474)
;; 1304.000000
+ 1554 1939 1529 1474 (- (- (* 2300 6) 6000) (+ 1554 1939 1529 1474))
;; 7800.000000
/ (+ 1554 1939 1529 1474 (- (- (* 2300 6) 6000) (+ 1554 1939 1529 1474)) 6000) 6
;; 2300.000000
- (- (* 2300 6) 6000) (+ 1554 1939 1529 2140 356)
;; 282.000000
cons 1
;; Error :  Function 'cons' passed incorrect number of arguments !
;;          expected 2, got 1.
cons 1 {4 5 5} 5
;; Error :  Function 'cons' passed incorrect number of arguments !
;;          expected 2, got 3.
cons 1 {4 5 5} 5 67 {4 6 7}
;; Error :  Function 'cons' passed incorrect number of arguments !
;;          expected 2, got 5.
cons {455 5 } 1
;; Error :  Function 'cons' passed incorrect type for element 1 !
;;          expected Q-Expression, got Number.
cons {455 5 } tail
;; Error :  Function 'cons' passed incorrect type for element 1 !
;;          expected Q-Expression, got Function.
cons {455 5 } ()
;; Error :  Function 'cons' passed incorrect type for element 1 !
;;          expected Q-Expression, got S-Expression.
list
;; <list>
list 1
;; {1.000000 }
list 1 {4 5 5}
;; {1.000000 {4.000000 5.000000 5.000000 } }
cons 1 {4 5 5}
;; {1.000000 4.000000 5.000000 5.000000 }
cons (+ 1 5.55) {4e44 5 5}
;; {6.550000 400000000000000035285445621225690562807463936.000000 5.000000 5.000000 }
cons (1) {4 5 5}
;; {1.000000 4.000000 5.000000 5.000000 }
cons {1} {4 5 5}
;; {{1.000000 } 4.000000 5.000000 5.000000 }
cons 5 (list 1 { 45 54 54 })
;; {5.000000 1.000000 {45.000000 54.000000 54.000000 } }
len 0 4
;; Error :  Function 'len' passed incorrect number of arguments !
;;          expected 1, got 2.
len (255)
;; Error :  Function 'len' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
len 1
;; Error :  Function 'len' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
len {54 4} {54 4}
;; Error :  Function 'len' passed incorrect number of arguments !
;;          expected 1, got 2.
len
;; <len>
len {54 4}
;; 2.000000
len ({54 4})
;; 2.000000
len {}
;; 0.000000
len {1}
;; 1.000000
len {1 (+ 45  54 ) {5454 54 56e64 -534.44e-7 }}
;; 3.000000
+ 4 (len {1 (+ 45  54 ) {5454 54 56e64 -534.44e-7 }})
;; 7.000000
+ 4 (len {1 (+ 45  54 ) {5454 54 56e64 -534.44e-7 }}) (eval{len { 7 4 5}})
;; 10.000000
len (cons 5 (list 1 { 45 54 54 }))
;; 3.000000
init 0 1 4
;; Error :  Function 'init' passed incorrect number of arguments !
;;          expected 1, got 3.
init 0
;; Error :  Function 'init' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
init (+ 0 1 4)
;; Error :  Function 'init' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
init {} (+ 0 1 4)
;; Error :  Function 'init' passed incorrect number of arguments !
;;          expected 1, got 2.
init {5 5} (* 0 1 4)
;; Error :  Function 'init' passed incorrect number of arguments !
;;          expected 1, got 2.
init {}
;; Error :  Function 'init' passed {} for element 0!
;; 
init {5 5} {0 1 4}
;; Error :  Function 'init' passed incorrect number of arguments !
;;          expected 1, got 2.
init
;; <init>
init {5 5}
;; {5.000000 }
init {5}
;; {}
init {5 (- 5 4 3 5)}
;; {5.000000 }
init {5 (- 5 4 3 5) 5}
;; {5.000000 (- 5.000000 4.000000 3.000000 5.000000 ) }
init {{5 7 52.5e5} (- 5 4 3 5) 5}
;; {{5.000000 7.000000 5250000.000000 } (- 5.000000 4.000000 3.000000 5.000000 ) }
init {(eval{len {* 7 4 5}}) {5 7 52.5e5} {(- 5 4 3 5) 5}}
;; {(eval {len {* 7.000000 4.000000 5.000000 } } ) {5.000000 7.000000 5250000.000000 } }
+ 4 (len {1 (+ 45  54 ) {5454 54 56e64 -534.44e-7 }}) (eval{len { 7 4 5}})
;; 10.000000
init {{5 7 52.5e5} {(- 5 4 3 5) 5} (eval{len { 7 4 5}})}
;; {{5.000000 7.000000 5250000.000000 } {(- 5.000000 4.000000 3.000000 5.000000 ) 5.000000 } }
eval {+ (len {eval(init{* 7 4 5})}) 5}
;; 7.000000
eval (head {5 10 11 15})
;; 5.000000
eval (head {+ - + - * /})
;; <add>
(eval (head {+ - + - * /})) 10 20
;; 30.000000
(eval (tail(tail(tail(tail(tail {+ - + - * /})))))) 10 20
;; 0.500000
hello
;; Error :  unbound symbol 'hello' !
add 3 4
;; 7.000000
+ 3 4
;; 7.000000
sub 5 2
;; 3.000000
- 5 2
;; 3.000000
mul 4 6
;; 24.000000
* 4 6
;; 24.000000
div 6 7
;; 0.857143
/ 6 7
;; 0.857143
div 6 0
;; Error :  Division by Zero !
/ 6 0
;; Error :  Division by Zero !
mod 6.35 4.21
;; 2.140000
% 6.35 4.21
;; 2.140000
pow 9 0
;; 1.000000
^ 9 0
;; 1.000000
pow 9 -4
;; 0.000152
^ 9 -4
;; 0.000152
> 546 5
;; 546.000000
max 546 5
;; 546.000000
min 546 5
;; 5.000000
< 546 5
;; 5.000000
head {+ 5 654 542 2.212 6.3e4}
;; {+ }
tail {+ 5 654 542 2.212 6.3e4}
;; {5.000000 654.000000 542.000000 2.212000 63000.000000 }
list 5 654 542 2.212 6.3e4
;; {5.000000 654.000000 542.000000 2.212000 63000.000000 }
init {+ 5 654 542 2.212 6.3e4}
;; {+ 5.000000 654.000000 542.000000 2.212000 }
eval {+ 5 654 542 2.212 6.3e4}
;; 64203.212000
join {+ 5 654 542 2.212 6.3e4} { 5 654 542 2.212 6.3e4}
;; {+ 5.000000 654.000000 542.000000 2.212000 63000.000000 5.000000 654.000000 542.000000 2.212000 63000.000000 }
cons {+ 5 654 542 2.212 6.3e4} { 5 654 542 2.212 6.3e4}
;; {{+ 5.000000 654.000000 542.000000 2.212000 63000.000000 } 5.000000 654.000000 542.000000 2.212000 63000.000000 }
len  {+ 5 654 542 2.212 6.3e4}
;; 6.000000
def 5
;; Error :  Function 'def' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
def 3 5
;; Error :  Function 'def' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
def {5 4} 4 5
;; Error :  Function 'def' passed incorrect type for element 0 !
;;          expected Symbol, got Number.
def {5 x} 4 5
;; Error :  Function 'def' passed incorrect type for element 0 !
;;          expected Symbol, got Number.
def {x 5} 4 5
;; Error :  Function 'def' passed incorrect type for element 1 !
;;          expected Symbol, got Number.
def {x y} 4
;; Error :  Function 'def' passed non-matching number of values and symbols !
;;          got 1 values and 2 symbols.
def {x } 4 5
;; Error :  Function 'def' passed non-matching number of values and symbols !
;;          got 2 values and 1 symbols.
def {x x dz len fg g} 4 5
;; Error :  Function 'def' passed non-matching number of values and symbols !
;;          got 2 values and 6 symbols.
def {x } 4 5 len 3 {+ 5 len (4)}
;; Error :  Function 'def' passed non-matching number of values and symbols !
;;          got 5 values and 1 symbols.
def {x } (4 5)
;; Error :  S-Expressions invalid first element type !
;;          expected Function, got Number.
x
;; Error :  unbound symbol 'x' !
x y
;; Error :  unbound symbol 'x' !
y z
;; Error :  unbound symbol 'y' !
def
;; <def>
def {x} 100
;; ()
def {y} 200
;; ()
x
;; 100.000000
y
;; 200.000000
+ x y
;; 300.000000
def {a b} 5 6
;; ()
+ a b
;; 11.000000
def {arglist} {a b x y}
;; ()
arglist
;; {a b x y }
def arglist 1 2 3 4
;; ()
list a b x y
;; {1.000000 2.000000 3.000000 4.000000 }
def {x } (+ 4 5)
;; ()
days
;; Error :  unbound symbol 'days' !
def {days} {1638 1515 2236 6000 1378 2504 2440 2349 2346 2210 2028}
;; ()
len days
;; 11.000000
def {current} {/ (eval (cons + days)) (len days)}
;; ()
def {target} {2300}
;; ()
target
;; {2300.000000 }
* (eval target) (len days)
;; 25300.000000
eval current
;; 2422.181818
+ (* (eval target) (+ (len days) 1)) (-(eval (cons + days)))
;; 956.000000
+ (* (eval target) 12) (-(eval (cons + days)))
;; 956.000000
def {days} 6
;; ()
/ (+ 1500 (* 2500 days)) 2300
;; 7.173913
head 4
;; Error :  Function 'head' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
head 4 5 4 {45 6}
;; Error :  Function 'head' passed incorrect number of arguments !
;;          expected 1, got 4.
head len {45 5}
;; Error :  Function 'head' passed incorrect number of arguments !
;;          expected 1, got 2.
head (len {45 5})
;; Error :  Function 'head' passed incorrect type for element 0 !
;;          expected Q-Expression, got Number.
head {}
;; Error :  Function 'head' passed {} for element 0!
head
;; <head>
tail
;; <tail>
list
;; <list>
init
;; <init>
eval
;; <eval>
join
;; <join>
cons
;; <cons>
len
;; <len>
def
;; <def>
sub
;; <sub>
mul
;; <mul>
add
;; <add>
div
;; <div>
mod
;; <mod>
pow
;; <pow>
max
;; <max>
min
;; <min>
+
;; <add>
-
;; <sub>
*
;; <mul>
/
;; <div>
%
;; <mod>
^
;; <pow>
>
;; <max>
<
;; <min>
