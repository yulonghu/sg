--TEST--
Check for Sg Associative array
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--FILE--
<?php
$key_one = 'user.a.a';
$val_one = 'A apple';

$key_two = 'user.a.b';
$val_two = 'B apple';

echo "------------------set\n";
var_dump(sg::set($key_one, $val_one));
var_dump(sg::set($key_two, $val_two));

echo "------------------get_user\n";
var_dump(sg::get('user'));

?>
--EXPECT--
------------------set
bool(true)
bool(true)
------------------get_user
array(1) {
  ["a"]=>
  array(2) {
    ["a"]=>
    string(7) "A apple"
    ["b"]=>
    string(7) "B apple"
  }
}

