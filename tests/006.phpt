--TEST--
Check for Sg::del 
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--FILE--
<?php

ini_set('sg.enable', 1);

$key_one = 'user.1.2.3.0';
$val_one = 'A apple';

$key_two = 'user.1.2.3.1';
$val_two = 'B apple';

echo "------------------set\n";
var_dump(sg::set($key_one, $val_one));
var_dump(sg::set($key_two, $val_two));

echo "------------------del\n";
var_dump(sg::del('user.1.2.3.0'));

echo "------------------get_user\n";
var_dump(sg::get('user'));

echo "------------------del_undefined\n";
var_dump(sg::del('undefined'));
var_dump(sg::del('undefined.0.1'));

?>
--EXPECT--
------------------set
bool(true)
bool(true)
------------------del
bool(true)
------------------get_user
array(1) {
  [1]=>
  array(1) {
    [2]=>
    array(1) {
      [3]=>
      array(1) {
        [1]=>
        string(7) "B apple"
      }
    }
  }
}
------------------del_undefined
bool(false)
bool(false)

