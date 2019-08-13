--TEST--
Check for sg.getCache()/set()/del()/getCacheAll()
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--FILE--
<?php

echo '-------------------default', PHP_EOL;

sg::set('id', 'start --- ' . '012345');

var_dump(sg::getCache('id'));
var_dump($GLOBALS['id']);
var_dump(sg::get('id'));

echo '-------------------GLOBALS change', PHP_EOL;
$GLOBALS['id'] = 'globals string --- ' . '123456';

var_dump($GLOBALS['id']);
var_dump(sg::getCache('id'));
var_dump(sg::get('id'));

echo '-------------------getCacheAll', PHP_EOL;
print_r(sg::getCacheAll());

echo '-------------------sg::set change', PHP_EOL;
sg::set('id', "set string --- " . '456789');

var_dump(sg::getCache('id'));
var_dump($GLOBALS['id']);
var_dump(sg::get('id'));

?>
--EXPECT--
-------------------default
string(16) "start --- 012345"
string(16) "start --- 012345"
string(16) "start --- 012345"
-------------------GLOBALS change
string(25) "globals string --- 123456"
string(16) "start --- 012345"
string(25) "globals string --- 123456"
-------------------getCacheAll
Array
(
    [id] => start --- 012345
)
-------------------sg::set change
string(21) "set string --- 456789"
string(21) "set string --- 456789"
string(21) "set string --- 456789"

