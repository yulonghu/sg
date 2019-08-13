--TEST--
Check for Global $var to change
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
sg.global_level = 1
--GET--
key=GET_test_key
--FILE--
<?php

function testGlobal()
{
    global $user, $g_key;

    var_dump($user, $g_key);
 
    $g_key = 'NEW_GET_test_key';
    $user = 'NEW_user';
}

function testGlobal_1()
{
    global $user, $g_key;

    var_dump($user, $g_key);
}

echo "------------------testGlobal()\n";

testGlobal();

echo "------------------testGlobal_1\n";

testGlobal_1();

echo "------------------sg And GLOBALS\n";

var_dump(sg::get('g.key'));
var_dump($GLOBALS['g_key']);
var_dump($g_key);
var_dump($_GET['key']);

echo "------------------_GET['key'] Change\n";

$_GET['key'] = 'GET_test_key_2';

var_dump(sg::get('g.key'));
var_dump($GLOBALS['g_key']);
var_dump($g_key);
var_dump($_GET['key']);

?>
--EXPECT--
------------------testGlobal()
NULL
string(12) "GET_test_key"
------------------testGlobal_1
string(8) "NEW_user"
string(16) "NEW_GET_test_key"
------------------sg And GLOBALS
string(16) "NEW_GET_test_key"
string(16) "NEW_GET_test_key"
string(16) "NEW_GET_test_key"
string(16) "NEW_GET_test_key"
------------------_GET['key'] Change
string(14) "GET_test_key_2"
string(14) "GET_test_key_2"
string(14) "GET_test_key_2"
string(14) "GET_test_key_2"

