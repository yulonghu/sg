--TEST--
Check for sg.global_level
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
sg.global_level = 0
--FILE--
<?php

$_GET['key']['key1']['key2'] = 'GET_test_key';

function testGlobal()
{
    global $g_key_key1_key2;
}

testGlobal();

var_dump(sg::get('g.key.key1.key2'));
var_dump($GLOBALS['g_key_key1_key2']);
var_dump($g_key_key1_key2);
var_dump($_GET['key']['key1']['key2']);

?>
--EXPECT--
string(12) "GET_test_key"
string(12) "GET_test_key"
string(12) "GET_test_key"
string(12) "GET_test_key"

