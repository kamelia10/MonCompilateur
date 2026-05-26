VAR
  a, b : INTEGER;
  x    : DOUBLE;
  c    : CHAR;
  ok   : BOOLEAN.

(* Test affectations et types *)
a := 10;
b := 3;
x := 2.5 + 1.5;
c := 'Z';
ok := a > b;

(* Test DISPLAY *)
DISPLAY a;
DISPLAY x;
DISPLAY c;

(* Test IF *)
IF ok THEN
  DISPLAY a
ELSE
  DISPLAY b;

(* Test FOR avec TO *)
FOR a := 1 TO 5 DO
  DISPLAY a;

(* Test WHILE *)
b := 0;
WHILE b < 3 DO
BEGIN
  b := b + 1;
  DISPLAY b
END;

(* Test CASE sur un entier *)
a := 2;
CASE a OF
  1 : DISPLAY a;
  2, 3 : DISPLAY b;
  4 : DISPLAY a
END.
