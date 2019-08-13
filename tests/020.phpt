--TEST--
Check for sg.func_name Example::decrypt
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--GET--
user=Hi, SG
--FILE--
<?php

ini_set('sg.func_name',  'Example::decrypt');

$_GET['user'] = base64_encode($_GET['user']);

class Example {
    public static function decrypt($data) {
        return base64_decode($data);
    }
}

var_dump(sg::get('g.user', 'def'));
var_dump(sg::get('found', 'dvalue_001'));

var_dump(sg::getCache('g.user', 'def'));
var_dump(sg::getCache('g.user', 'def'));

$data = sg::getCache('g.user');
unset($data);

$data = sg::getCache('g.user');
var_dump($data);
?>
--EXPECT--
string(6) "Hi, SG"
string(10) "dvalue_001"
string(6) "Hi, SG"
string(6) "Hi, SG"
string(6) "Hi, SG"

