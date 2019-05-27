--TEST--
Check for global $var refcount
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
sg.auto_trim = 1
--FILE--
<?php

$a = ['a', 'b'];
$b = & $a;

$_GET['key'] = & $b;

function testGlobal()
{
    global $g_key;
}

testGlobal();

var_dump($g_key);
var_dump(sg::get('g.key2', $b));

?>
--EXPECT--
array(2) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
}
array(2) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
}

