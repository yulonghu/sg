# 一个简单的PHP语法糖扩展

[![Build Status](https://travis-ci.org/yulonghu/sg.svg?branch=master)](https://travis-ci.org/yulonghu/sg)

[English Document](https://github.com/yulonghu/sg/blob/master/README.md) | 中文

### Table of Contents

* [介绍](#介绍)
* [特性](#特性)
* [安装](#安装)
    * [支持PHP版本](#支持php版本)
    * [下载源码](#下载源码)
    * [Linux系统，编译SG扩展](#linux系统编译sg扩展)
        * [php.ini，添加 & 开启扩展](#phpini添加--开启扩展)
* [帮助文档](#帮助文档)
    * [API](#api)
        * [global 声明方式](#global-声明方式)
        * [封装的静态方法](#封装的静态方法)
    * [配置项(php.ini)](#配置项phpini)
    * [Hash Map](#hash-map)
    * [例子](#例子)
        * [global 声明例子](#global-声明例子)
            * [sg.global_level = 1](#sgglobal_level--1)
            * [sg.global_level = 0](#sgglobal_level--0)
            * [sg.func_name](#sgfunc_name)
        * [静态方法例子](#静态方法例子)
            * [sg::get/set/has/del()](#sggetsethasdel)
            * [array sg::all(void)](#array-sgallvoid)
            * [sg.func_name](#sgfunc_name-1)
* [License](#license)

### 介绍

[SG](https://github.com/yulonghu/sg) 全称 [Superglobals](http://php.net/manual/en/language.variables.superglobals.php)，引用全局作用域中可用的全部变量。SG丰富了PHP超全局变量获取方式。

这些超全局变量有：$_SERVER，$_GET，$_POST，$_FILES，$_COOKIE，$_SESSION，$_REQUEST，$_ENV。

当然了，也可以应用到自定义变量场景。

非常重要的一点：它很简单！

### 特性
- 简单，快速，轻量
- 零拷贝访问PHP超全局变量，使用SG会同步更新PHP超全局变量
- 支持取值前调用自定义函数，默认情况下，字符串变量会自动调用 [PHP trim](http://php.net/manual/en/function.trim.php)
- 解决使用PHP超全局变量时出现未定义系列的问题 (Undefined variable, Undefined offset)
- 采用函数调用方式时，以小数点代替PHP数组维度
- 采用global声明方式时，以下划线代替PHP数组维度
- 支持可配置的global $variable查找深度，默认一级查找

## 安装
### 支持PHP版本
- PHP 5.4 +
- PHP 7.0 +

### 下载源码
```
git clone https://github.com/yulonghu/sg.git
```

### Linux系统，编译SG扩展
```
$ /path/to/php/bin/phpize
$ ./configure --with-php-config=/path/to/php/bin/php-config
$ make && make install
```

#### php.ini，添加 & 开启扩展
```ini
extension=sg.so

[sg]
sg.enable = On
```

重启php进程，就安装成功啦。

## 帮助文档

### API

#### global 声明方式

```php
global $g_key, $p_key, $c_key, $s_key, $f_key, $n_key, $e_key, $r_key
```

#### 封装的静态方法
```php
array sg::all(void)

// 读取全局作用域变量, 找不到$key时，$default_value值生效
mixed sg::get(string $key [, mixed $default_value = null])

// 设置全局作用域变量，找不到$key则新增，否则更新其值; 返回值: TRUE 成功，FALSE 失败
bool sg::set(string $key, mixed $value)

// 检查全局作用域中$key是否存在; 返回值: TRUE 存在，FALSE 不存在
bool sg::has(string $key)

// 删除全局作用域中变量$key; 返回值: TRUE 成功，FALSE 失败
// 注意: 批量删除时，有且只有一个成功结果
bool sg::del(string $key [, mixed $... ])
```

### 配置项(php.ini)
| 配置项 | 权限 | 类型 |默认值 | 说明 |
|---|---|---|---|---|
|sg.enable|PHP_INI_SYSTEM|bool|0| 0 关闭 1 开启|
|sg.global_level|PHP_INI_SYSTEM|bool|1| 1 只支持一级查找 0 无限制查找|
|sg.func_name|PHP_INI_ALL|char|trim|默认调用 [PHP trim](http://php.net/manual/en/function.trim.php) 函数，也支持自定义函数 |

### Hash Map
- 管理PHP会话时(MapKey = n), 请先调用函数 session_start()

|PHP 超全局变量|SG key (关键字缩写)|global 声明|函数|
| ------ | ------ | ------ | ------ |
|$GLOBALS|无|无|sg::all()|
|$_SERVER|s|global $s|sg::get/set/has/del('s')|
|$_GET|g|global $g|sg::get/set/has/del('g')|
|$_POST|p|global $p|sg::get/set/has/del('p')|
|$_FILES|f|global $f|sg::get/set/has/del('f')|
|$_COOKIE|c|global $c|sg::get/set/has/del('c')|
|$_SESSION|n|global $n|sg::get/set/has/del('n')|
|$_REQUEST|r|global $r|sg::get/set/has/del('r')|
|$_ENV|e|global $e|sg::get/set/has/del('e')|

### 例子

#### global 声明例子
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

以上例子输出的结果：

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

以上例子输出的结果：

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

var_dump($p_key);
```

以上例子输出的结果：

```txt
string(8) "A Banana"
```

#### 静态方法例子
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

以上例子输出的结果:

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
结果与 [$GLOBALS](https://www.php.net/manual/zh/reserved.variables.globals.php) 相同

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

以上例子输出的结果：

```txt
string(8) "A Banana"
```

## License
SG is open source software under the [PHP License v3.01](http://www.php.net/license/3_01.txt)

