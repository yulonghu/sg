--TEST--
Check for sg.get() and var_dump($_GET, $_POST)
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
sg.global_level = 1
--GET--
ekey=A Banana
--POST--
dkey=A Apple
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

echo "------------------print source _POST\n";

var_dump($_POST);

echo "------------------decryptTest\n";

var_dump(sg::get('p.dkey'));

echo "------------------print result _POST\n";
var_dump($_POST);

echo "\n------------------encryptTest\n";

ini_set('sg.func_name', 'encryptTest');

var_dump(sg::get('g.ekey'));

echo "------------------print _GET\n";
var_dump($_GET);

?>
--EXPECT--
------------------print source _POST
array(1) {
  ["dkey"]=>
  string(12) "QSBBcHBsZQ=="
}
------------------decryptTest
string(7) "A Apple"
------------------print result _POST
array(1) {
  ["dkey"]=>
  string(12) "QSBBcHBsZQ=="
}

------------------encryptTest
string(12) "QSBCYW5hbmE="
------------------print _GET
array(1) {
  ["ekey"]=>
  string(8) "A Banana"
}

