--TEST--
Check for Global $var ...
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
sg.global_level = 1
--FILE--
<?php

/* Init Variable */

$g_user = '101';
$g_var  = 'hello' . 'world';

/* Init Superglobals */

$_SERVER['key']     = 'SERVER_test_key';
$_GET['key']        = 'GET_test_key';
$_POST['key']       = 'POST_test_key';
$_FILES['key']      = 'FILES_test_key';
$_COOKIE['key']     = 'COOKIE_test_key';
//$_SESSION['key']  = 'SESSION_test_key';
$_REQUEST['key']    = 'REQUEST_test_key';
$_ENV['key']        = 'ENV_test_key';

/* Will not get value from $_GET */
$_GET['user']   = 'have_user';
$_GET['var']    = 'have_var';

function testGlobal()
{
    global $s_key, $g_key, $p_key, $f_key, $c_key, $r_key, $e_key;
    global $g_user, $g_var;

    var_dump($s_key, $g_key, $p_key, $f_key, $c_key, $r_key, $e_key);
    var_dump($g_user, $g_var);
}

echo "------------------testGlobal()\n";

testGlobal();

echo "------------------Global\n";

global $s_key, $g_key, $p_key, $f_key, $c_key, $r_key, $e_key;
global $g_user, $g_var;

var_dump($s_key, $g_key, $p_key, $f_key, $c_key, $r_key, $e_key);
var_dump($g_user, $g_var);

?>
--EXPECT--
------------------testGlobal()
string(15) "SERVER_test_key"
string(12) "GET_test_key"
string(13) "POST_test_key"
string(14) "FILES_test_key"
string(15) "COOKIE_test_key"
string(16) "REQUEST_test_key"
string(12) "ENV_test_key"
string(3) "101"
string(10) "helloworld"
------------------Global
string(15) "SERVER_test_key"
string(12) "GET_test_key"
string(13) "POST_test_key"
string(14) "FILES_test_key"
string(15) "COOKIE_test_key"
string(16) "REQUEST_test_key"
string(12) "ENV_test_key"
string(3) "101"
string(10) "helloworld"

