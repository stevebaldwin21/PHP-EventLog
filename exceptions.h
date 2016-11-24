#pragma once

#define EXCEPTION_NS(cls) "Windows\\Exceptions\\" ##cls

zend_class_entry ce;
zend_class_entry * windows_access_denied;
zend_class_entry * windows_invalid_operation_exception;
zend_class_entry * windows_invalid_file_exception;
zend_class_entry * windows_out_of_bounds_exception;
zend_class_entry * windows_runtime_exception;
zend_class_entry * windows_invalid_argument_exception;
zend_class_entry * windows_illegal_operation_exception;


void init_windows_exceptions();
void throw_windows_exception(zend_class_entry * class);
void windows_throw_exception(zend_class_entry * class, char * message);
void windows_throw_exception_hres(DWORD hResult);
