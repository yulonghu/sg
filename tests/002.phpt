--TEST--
Check for Sg All Method
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--FILE--
<?php
$key = 'test';
$val = 'A apple';

echo "------------------start\n";
var_dump(sg::get($key));
var_dump(sg::get($key, 'def'));
var_dump(sg::has($key));

echo "------------------set\n";
var_dump(sg::set($key, $val));

echo "------------------get\n";
var_dump(sg::get($key));
var_dump(sg::get($key, 'def'));
var_dump(sg::has($key));

echo "------------------del\n";
var_dump(sg::del($key));

echo "------------------get\n";
var_dump(sg::get($key));
var_dump(sg::has($key));

echo "------------------others\n";
var_dump(sg::set('others.', 100));
var_dump(sg::get('others'));

echo "------------------IS_TMP_VAR\n";
var_dump(sg::set('var', ' hello' . time()));
var_dump(sg::set('var.abc.def', "hello_word" . time()));

sg::all();

?>
--EXPECT--
------------------start
NULL
string(3) "def"
bool(false)
------------------set
bool(true)
------------------get
string(7) "A apple"
string(7) "A apple"
bool(true)
------------------del
bool(true)
------------------get
NULL
bool(false)
------------------others
bool(true)
int(100)
------------------IS_TMP_VAR
bool(true)
bool(true)

