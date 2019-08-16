--TEST--
Check for sg.set()/get() global variable
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--FILE--
<?php

$a = 'hello';
$b = 'world';
$c = $a . $b;

sg::set('user', $c);
sg::set('list.sinfo', 'info');
sg::set('list.finfo', 0.123456);

function one() {
    sg::set('id', 1509697);
}

function two() {
    var_dump(sg::get('user'));
    var_dump(sg::get('id'));
}

var_dump(sg::get('list'));

one();
two();

?>
--EXPECT--
array(2) {
  ["sinfo"]=>
  string(4) "info"
  ["finfo"]=>
  float(0.123456)
}
string(10) "helloworld"
int(1509697)

