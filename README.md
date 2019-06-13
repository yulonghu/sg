# SG - PHP Syntax Sugar Extension

[![Build Status](https://travis-ci.org/yulonghu/sg.svg?branch=master)](https://travis-ci.org/yulonghu/sg)

[中文文档](https://github.com/yulonghu/sg/blob/master/README_ZH.md) | English Document

### Table of Contents

* [Introduction](#introduction)
* [Features](#features)
* [Install](#install)
    * [Supported Version](#supported-version)
    * [DownLoad](#download)
    * [Compile SG in Linux](#compile-sg-in-linux)
        * [Add the follow information to your php.ini](#add-the-follow-information-to-your-phpini)
* [Help Manual](#help-manual)
    * [API](#api)
        * [global $variable](#global-variable)
        * [Static Methods](#static-methods)
    * [Inis(php.ini)](#inisphpini)
    * [Hash Map](#hash-map)
    * [Example](#example)
        * [global $variable](#global-variable-1)
            * [sg.global_level = 1](#sgglobal_level--1)
            * [sg.global_level = 0](#sgglobal_level--0)
            * [sg.func_name](#sgfunc_name)
        * [Static Methods](#static-methods-1)
            * [sg::get/set/has/del()](#sggetsethasdel)
            * [array sg::all(void)](#array-sgallvoid)
            * [sg.func_name](#sgfunc_name-1)
* [License](#license)

### Introduction

[SG](https://github.com/yulonghu/sg) Full name [Superglobals](http://php.net/manual/en/language.variables.superglobals.php), References all variables available in global scope, SG has extended a new way to manage PHP superglobals variables, Make the management of PHP superglobals variables simple and unified.

These superglobal variables managed by SG are: $_SERVER, $_GET, $_POST, $_FILES, $_COOKIE, $_SESSION, $_REQUEST, $_ENV.

Very important point: it is very simple.

### Features
- Simple, Fast, Lightweight
- Access PHP superglobals zero-copy, Synchronously update PHP Superglobals variables
- Support custom call function name, Default call [PHP trim](http://php.net/manual/en/function.trim.php)
- Solve the problem of undefined series when using PHP Superglobals variables (Undefined variable, Undefined offset)
- Use static function method, Replace the PHP array dimension with a decimal point
- Use global statement, Replace the PHP array dimension with a underline
- Support for global $variable the option configuration, Default level one lookup

## Install
### Supported Version
- PHP 5.4 +
- PHP 7.0 +

### DownLoad
```
git clone https://github.com/yulonghu/sg.git
```

### Compile SG in Linux
```
$ /path/to/php/bin/phpize
$ ./configure --with-php-config=/path/to/php/bin/php-config
$ make && make install
```

#### Add the follow information to your php.ini
```ini
extension=sg.so

[sg]
sg.enable = On
```

Restart the php-fpm.

## Help Manual

### API

#### global $variable

```php
global $g_key, $p_key, $c_key, $s_key, $f_key, $n_key, $e_key, $r_key
```

#### Static Methods
```php
array sg::all(void)
mixed sg::get(string $key [, mixed $default_value = null])
bool sg::set(string $key, mixed $value)
bool sg::has(string $key)
bool sg::del(string $key [, mixed $... ])
```

### Inis(php.ini)
| Options | Permission | Type | Default | Desc |
|---|---|---|---|---|
|sg.enable|PHP_INI_SYSTEM|bool|0| 0 Trun-Off 1 Turn-On|
|sg.global_level|PHP_INI_SYSTEM|bool|1| 1 Limit Level 0 Unlimited Level|
|sg.func_name|PHP_INI_ALL|char|trim| [PHP trim](http://php.net/manual/en/function.trim.php), Support for custom function|

### Hash Map
- When management PHP sessions (MapKey = n), First call the function session_start()

|PHP Predefined Superglobals|SG Key|global statement| Method Example|
| ------ | ------ | ------ | ------ |
|$GLOBALS|-|-|sg::all()|
|$_SERVER|s|global $s|sg::get/set/has/del('s')|
|$_GET|g|global $g|sg::get/set/has/del('g')|
|$_POST|p|global $p|sg::get/set/has/del('p')|
|$_FILES|f|global $f|sg::get/set/has/del('f')|
|$_COOKIE|c|global $c|sg::get/set/has/del('c')|
|$_SESSION|n|global $n|sg::get/set/has/del('n')|
|$_REQUEST|r|global $r|sg::get/set/has/del('r')|
|$_ENV|e|global $e|sg::get/set/has/del('e')|

### Example

#### global $variable
##### sg.global_level = 1

```php
<?php

$_GET['key'] = 'GET_test_key';

function testGlobal()
{
    global $g_key;

    var_dump($g_key);

    $g_key = 'NEW_GET_test_key';
}

testGlobal();

var_dump(sg::get('g.key'));
var_dump($GLOBALS['g_key']);
var_dump($g_key);
var_dump($_GET['key']);
```

The above example will output:

```txt
string(12) "GET_test_key"
string(16) "NEW_GET_test_key"
string(16) "NEW_GET_test_key"
string(16) "NEW_GET_test_key"
string(16) "NEW_GET_test_key"
```

##### sg.global_level = 0

```php
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
```

The above example will output:

```txt
string(12) "GET_test_key"
string(12) "GET_test_key"
string(12) "GET_test_key"
string(12) "GET_test_key"
```
##### sg.func_name

```php
<?php

ini_set('sg.func_name', 'decryptTest');

$_POST['key'] = 'IEEgQmFuYW5hIA==';

function decryptTest($data)
{
    return trim(base64_decode($data));
}

global $p_key;

var_dump($p_key);
```

The above example will output:

```txt
string(8) "A Banana"
```

#### Static Methods
##### sg::get/set/has/del()

```php
<?php

$key = 'test';
$val = 'A Banana';

echo "------------------start\n";
var_dump(sg::get($key));
var_dump(sg::get($key, 'def'));
var_dump(sg::has($key));

echo "------------------set\n";
var_dump(sg::set($key, $val));

echo "------------------get\n";
var_dump(sg::get($key));
var_dump(sg::get($key, 'def'));
var_dump(sg::has($key));

echo "------------------del\n";
var_dump(sg::del($key));

echo "------------------get\n";
var_dump(sg::get($key));
var_dump(sg::has($key));
```

The above example will output:

```txt
------------------start
NULL
string(3) "def"
bool(false)
------------------set
bool(true)
------------------get
string(8) "A banana"
string(8) "A banana"
bool(true)
------------------del
bool(true)
------------------get
NULL
bool(false)
```

##### array sg::all(void)
Same as [$GLOBALS](https://www.php.net/manual/zh/reserved.variables.globals.php)

##### sg.func_name
```php
<?php

ini_set('sg.func_name', 'decryptTest');

function decryptTest($data)
{
    return trim(base64_decode($data));
}

function encryptTest($data) 
{
    return base64_encode(trim($data));
}

sg::set('user', encryptTest(' A Banana '));
var_dump(sg::get('user'));
```

The above example will output:

```txt
string(8) "A Banana"
```

## License
SG is open source software under the [PHP License v3.01](http://www.php.net/license/3_01.txt)

