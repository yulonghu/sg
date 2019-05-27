--TEST--
Check for sg.auto_trim
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
sg.auto_trim = 1
--FILE--
<?php
$key = 'test';
$val = ' A apple ';

var_dump(sg::set($key, $val));
var_dump(sg::get($key));

$_POST['key'] = ' A book ';
global $p_key;

var_dump($GLOBALS['p_key']);
var_dump($p_key);
var_dump($_POST['key']);
var_dump(sg::get('p.key'));

?>
--EXPECT--
bool(true)
string(7) "A apple"
string(6) "A book"
string(6) "A book"
string(6) "A book"
string(6) "A book"

