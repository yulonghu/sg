--TEST--
Check for sg.func_name
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
sg.global_level = 1
--FILE--
<?php

ini_set('sg.func_name', 'decryptTest');

function decryptTest($data)
{
    return trim(base64_decode($data));
}

function encryptTest($data) 
{
    return base64_encode($data);
}

$_POST['dkey'] = 'QSBBUFBMRQ==';
$_POST['dpass'] = 'MTIzNDU2';

echo "------------------decryptTest\n";

var_dump(sg::get('p.dkey'));

global $p_dpass;
var_dump($p_dpass);

echo "------------------encryptTest\n";

ini_set('sg.func_name', 'encryptTest');

$_GET['ekey'] = 'A Banana';
var_dump(sg::get('g.ekey'));
var_dump(sg::get('g.ekey'));

$_GET['epass'] = '456789';
global $g_epass;
var_dump($g_epass);

global $g_epass;
var_dump($g_epass);

?>
--EXPECT--
------------------decryptTest
string(7) "A APPLE"
string(6) "123456"
------------------encryptTest
string(12) "QSBCYW5hbmE="
string(16) "UVNCQ1lXNWhibUU9"
string(8) "NDU2Nzg5"
string(8) "NDU2Nzg5"

