--TEST--
Check for sg.func_name repeat get value
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
sg.global_level = 1
--POST--
dkey=A Apple&dpass=123456
--GET--
ekey=A Apple&epass=123456
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

$_POST['dkey'] = base64_encode($_POST['dkey']);
$_POST['dpass'] = base64_encode($_POST['dpass']);

echo "------------------decryptTest\n";

var_dump($_POST['dkey'], $_POST['dpass']);

var_dump(sg::get('p.dkey'));
var_dump(sg::get('p.dkey'));
var_dump(sg::get('p.dkey'));

global $p_dpass;

var_dump($p_dpass);
var_dump($p_dpass);

global $p_dpass;

var_dump($p_dpass);
var_dump($p_dpass);


echo "------------------encryptTest\n";

ini_set('sg.func_name', 'encryptTest');

var_dump(sg::get('g.ekey'));
var_dump(sg::get('g.ekey'));

global $g_epass;

var_dump($g_epass);
var_dump($g_epass);

global $g_epass;

var_dump($g_epass);
var_dump($g_epass);
?>
--EXPECT--
------------------decryptTest
string(12) "QSBBcHBsZQ=="
string(8) "MTIzNDU2"
string(7) "A Apple"
string(7) "A Apple"
string(7) "A Apple"
string(6) "123456"
string(6) "123456"
string(6) "123456"
string(6) "123456"
------------------encryptTest
string(12) "QSBBcHBsZQ=="
string(12) "QSBBcHBsZQ=="
string(8) "MTIzNDU2"
string(8) "MTIzNDU2"
string(8) "MTIzNDU2"
string(8) "MTIzNDU2"

