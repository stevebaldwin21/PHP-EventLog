#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_vm.h"
#include "zend_exceptions.h"

/*
* Copyright (c) 2016 Steve Baldwin <stevebaldwin21@googlemail.com>
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
* and to permit persons to whom the Software is furnished to do so, subject to the following conditions;
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* see https://wiki.php.net/rfc/fast_zpp
* 
* @author Steve Baldwin <stevebaldwin21@googlemail.com>
* Report bugs to the above email address
*/


#define GET_PROPERTY(c, s) { ZVAL_COPY(return_value, zend_read_property(c, getThis(), s, strlen(s), 0, 0), 0, 1); }
#define GET_STATIC_PROPERTY(c, s) { ZVAL_COPY(return_value, zend_read_static_property(c, s, strlen(s), 0, 0), 0, 1); }
#define RETURN_THIS { ZVAL_COPY(return_value, getThis() ); }

#define EVENTLOG_NS(cls) "Windows\\Diagnostics\\" ##cls
#define EVENTLOG_EXCEPTION_NS(cls) EVENTLOG_NS("Exception\\") ##cls

zend_class_entry ce;
zend_class_entry * eventlog_log_class;
zend_class_entry * event_class;
zend_class_entry * error_event_class;

PHP_METHOD(EventLog, writeEntry);
PHP_METHOD(EventLog, setSource);
PHP_METHOD(EventLog, getSource);


/* Iterates the hashtable values looking for a match */
bool zend_ht_val_exists_long(HashTable * HT, long val)
{
	zval * zIterator;
	ZEND_HASH_FOREACH_VAL(HT, zIterator);
	if (Z_LVAL_P(zIterator), val) {
		zIterator = NULL;
		efree(zIterator);
		return true;
	}
	ZEND_HASH_FOREACH_END();
	zIterator = NULL;
	efree(zIterator);
	return false;
}

