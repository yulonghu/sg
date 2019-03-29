--TEST--
Check for Sg Index array
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--FILE--
<?php
$key_one = 'user.0.0';
$val_one = 'A apple';

$key_two = 'user.0.1';
$val_two = 'B apple';

echo "------------------set\n";
var_dump(sg::set($key_one, $val_one));
var_dump(sg::set($key_two, $val_two));

echo "------------------get_user\n";
var_dump(sg::get('user'));


echo "------------------set_other\n";
var_dump(sg::set('c.d.e', array('d' => 1)));
var_dump(sg::set('c.d.e.f', 2));

echo "------------------get_other\n";
var_dump(sg::get('c.d.e'));

?>
--EXPECT--
------------------set
bool(true)
bool(true)
------------------get_user
array(1) {
  [0]=>
  array(2) {
    [0]=>
    string(7) "A apple"
    [1]=>
    string(7) "B apple"
  }
}
------------------set_other
bool(true)
bool(true)
------------------get_other
array(2) {
  ["d"]=>
  int(1)
  ["f"]=>
  int(2)
}
