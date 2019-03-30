# SG - PHP Superglobals Management
[![Build Status](https://travis-ci.org/yulonghu/sg.svg?branch=master)](https://travis-ci.org/yulonghu/sg)

A Simple PHP Superglobals Management

[中文文档](https://github.com/yulonghu/sg/blob/master/README_ZH.md)

### Introduction

SG Full name [Superglobals](http://php.net/manual/en/language.variables.superglobals.php), It is very easy to management PHP predefined Superglobals variables and User-defined Superglobals variables.

If in non CLI mode, SG default registration Superglobals include $_GET, $_POST, $_COOOKIE, $_SERVER, $_FILES.

Use SG can save a lot of PHP code, Improve development efficiency.

### Features
- Simple, Fast, Lightweight
- Access global variable zero-copy
- Strip whitespace with [PHP trim](http://php.net/manual/en/function.trim.php)
- Solve problems with PHP undefined series (Undefined variable, Undefined offset)

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

## Methods
```php
mixed sg::get(string $key [, mixed $default_value = null])
bool sg::set(string $key, mixed $value)
bool sg::has(string $key)
bool sg::del(string $key)
```

## Inis
```ini
sg.enable = On/Off
sg.auto_trim = On/Off ; Strip whitespace with PHP trim
```
## Example

### Get PHP Predefined Superglobals variable

|OLD GET METHOD (Short)|NEW GET METHOD|
| ------ | ------ |
|$_GET['key']|sg::get('g.key')|
|$_POST['key']|sg::get('p.key')|
|$_COOKIE['key']|sg::get('c.key')|
|$_SERVER['key']|sg::get('s.key')|
|$_FILES['key']|sg::get('f.key')|

|OLD GET METHOD (Long)|NEW GET METHOD|
| ------ | ------ |
|$_GET['key']['key1']['key2']|sg::get('g.key.key1.key2')|
|$_POST['key']['key1']['key2']|sg::get('p.key.key1.key2')|
|$_COOKIE['key']['key1']['key2']|sg::get('c.key.key1.key2')|
|$_SERVER['key']['key1']['key2']|sg::get('s.key.key1.key2')|
|$_FILES['key']['key1']['key2']|sg::get('f.key.key1.key2')|

### Get User-defined Superglobal variables

#### bool sg::set(string $key, mixed $value)
```php
<?php
var_dump(sg::set('test', 'test apple'));
var_dump(sg::set('user.0.0', 'user 0 apple'));
var_dump(sg::set('user.0.1', 'user 1 apple'));
var_dump(sg::set('user.a.a', 'user a apple'));
var_dump(sg::set('user.a.b', 'user b apple'));
```
The above example will output:
```txt
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
```

#### mixed sg::get(string $key [, mixed $default_value = null])
```php
<?php
var_dump(sg::get('test', 'test apple'));
var_dump(sg::get('user');
var_dump(sg::get('not_found', 'def');
var_dump(sg::get('user.1.2.3.4'));
```
The above example will output:
```txt
string(10) "test apple"
array(2) {
  [0]=>
  array(2) {
    [0]=>
    string(12) "user 0 apple"
    [1]=>
    string(12) "user 1 apple"
  }
  ["a"]=>
  array(2) {
    ["a"]=>
    string(12) "user a apple"
    ["b"]=>
    string(12) "user b apple"
  }
}
string(3) "def"
NULL
```

#### bool sg::has(string $key)
```php
<?php
var_dump(sg::has('test'));
var_dump(sg::has('not_found'));
```
The above example will output:
```
bool(true)
bool(false)
```

#### bool sg::del(string $key)
```php
<?php
var_dump(sg::del('test'));
var_dump(sg::del('user.0.1'));
var_dump(sg::get('user');
```
The above example will output:
```
bool(true)
bool(true)
array(2) {
  [0]=>
  array(1) {
    [0]=>
    string(12) "user 0 apple"
  }
  ["a"]=>
  array(2) {
    ["a"]=>
    string(12) "user a apple"
    ["b"]=>
    string(12) "user b apple"
  }
}
```

### sg.auto_trim

```php
<?php
// sg.auto_trim = On ; php.ini
ini_set('sg.auto_trim', 1);
function One() {
    var_dump(sg::set('test', ' test apple ')); // Auto-call PHP trim
}
function Two() {
    var_dump(sg::get('test'));
}
```
The above example will output:
```txt
bool(true)
string(10) "test apple"
```

## License
SG is open source software under the [PHP License v3.01](http://www.php.net/license/3_01.txt)

