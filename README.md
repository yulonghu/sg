# SG - PHP Superglobals Management
[![Build Status](https://travis-ci.org/yulonghu/sg.svg?branch=master)](https://travis-ci.org/yulonghu/sg)

A Simple PHP Superglobals Management

### Introduction

SG is the same as PHP [Superglobals](http://php.net/manual/en/language.variables.superglobals.php), Management global variables are $_GET, $_POST, $_COOOKIE, $_SERVER, $_FILES, User-defined global variables.

### Features
- Simple, Fast, Lightweight
- Access global variable zero-copy
- Strip whitespace with [PHP trim](http://php.net/manual/en/function.trim.php)
- Solve problems with PHP undefined series (Undefined variable, Undefined offset)

## Install
### Requirements
- PHP 7.0 +

### DownLoad
```
git clone https://github.com/yulonghu/sg.git
```

### Compile SG in Linux
```
$ /path/to/php7/bin/phpize
$ ./configure --with-php-config=/path/to/php7/bin/php-config
$ make && make install
```

#### Add the follow information to your php.ini
```ini
extension=sg.so

[sg]
sg.enable = On
```

Restart the web server

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

### PHP Superglobals

|OLD GET METHOD|NEW GET METHOD|
| ------ | ------ |
|$_GET['key']|sg::get('g.key')|
|$_POST['key']|sg::get('p.key')|
|$_COOKIE['key']|sg::get('c.key')|
|$_SERVER['key']|sg::get('s.key')|
|$_FILES['key']|sg::get('f.key')|

### User-defined global variables

#### sg::set
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

#### sg::get
```php
<?php
var_dump(sg::get('test', 'test apple'));
var_dump(sg::get('user');
var_dump(sg::get('not_found', 'def');
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
```
#### sg::has
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
#### sg::del
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

## License
SG is open source software under the [PHP License v3.01](http://www.php.net/license/3_01.txt)

