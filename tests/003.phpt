--TEST--
Check for Sg string key
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--FILE--
<?php
$key = 'user.list.product';
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

echo "------------------get_not_found\n";
$key = 'not.found';
var_dump(sg::get($key));
var_dump(sg::get($key, 'def'));
var_dump(sg::has($key));

echo "------------------trim\n";
$val = 'hello ';
sg::set($key, ' trim_' . $val);
var_dump(sg::get($key));

unset($name);

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
------------------get_not_found
NULL
string(3) "def"
bool(false)
------------------trim
string(10) "trim_hello"

