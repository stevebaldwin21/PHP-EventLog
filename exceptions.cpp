#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_vm.h"
#include "zend_exceptions.h"
#include "exceptions.h"

LPTSTR get_last_error_str(DWORD Error) 
{
	LPTSTR errorText = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		Error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&errorText,
		0,
		NULL
	);
	return errorText;
}

/* Loads windows specific exceptions */
void init_windows_exceptions() {

	INIT_CLASS_ENTRY(ce, EXCEPTION_NS("InvalidOperationException"), NULL);
	windows_invalid_operation_exception = zend_register_internal_class_ex(&ce, zend_ce_exception);
	windows_invalid_operation_exception->ce_flags |= ZEND_ACC_FINAL;
	windows_invalid_operation_exception->ce_flags |= ZEND_ACC_ABSTRACT;

	INIT_CLASS_ENTRY(ce, EXCEPTION_NS("UnauthorizedAccessException"), NULL);
	windows_access_denied = zend_register_internal_class_ex(&ce, zend_ce_exception);
	windows_access_denied->ce_flags |= ZEND_ACC_FINAL;
	windows_access_denied->ce_flags |= ZEND_ACC_ABSTRACT;

	INIT_CLASS_ENTRY(ce, EXCEPTION_NS("InvalidFileException"), NULL);
	windows_invalid_file_exception = zend_register_internal_class_ex(&ce, zend_ce_exception);
	windows_invalid_file_exception->ce_flags |= ZEND_ACC_FINAL;
	windows_invalid_file_exception->ce_flags |= ZEND_ACC_ABSTRACT;

	INIT_CLASS_ENTRY(ce, EXCEPTION_NS("OutOfBoundsException"), NULL);
	windows_out_of_bounds_exception = zend_register_internal_class_ex(&ce, zend_ce_exception);
	windows_out_of_bounds_exception->ce_flags |= ZEND_ACC_FINAL;
	windows_out_of_bounds_exception->ce_flags |= ZEND_ACC_ABSTRACT;

	INIT_CLASS_ENTRY(ce, EXCEPTION_NS("RuntimeException"), NULL);
	windows_runtime_exception = zend_register_internal_class_ex(&ce, zend_ce_exception);
	windows_runtime_exception->ce_flags |= ZEND_ACC_FINAL;
	windows_runtime_exception->ce_flags |= ZEND_ACC_ABSTRACT;

	INIT_CLASS_ENTRY(ce, EXCEPTION_NS("InvalidArgumentException"), NULL);
	windows_invalid_argument_exception = zend_register_internal_class_ex(&ce, zend_ce_exception);
	windows_invalid_argument_exception->ce_flags |= ZEND_ACC_FINAL;
	windows_invalid_argument_exception->ce_flags |= ZEND_ACC_ABSTRACT;

	INIT_CLASS_ENTRY(ce, EXCEPTION_NS("IllegalOperationException"), NULL);
	windows_illegal_operation_exception = zend_register_internal_class_ex(&ce, zend_ce_exception);
	windows_illegal_operation_exception->ce_flags |= ZEND_ACC_FINAL;
	windows_illegal_operation_exception->ce_flags |= ZEND_ACC_ABSTRACT;
}


void throw_windows_exception(zend_class_entry * class)
{
	DWORD hResult = GetLastError();
	zend_throw_exception_ex(class, hResult, get_last_error_str(hResult));
	return;
}
/* Throws a zend exception using zend_throw_exception_ex() finds the string message by DWORD response */
void windows_throw_exception_hres(DWORD errorCode)
{
	//default ce exception
	zend_class_entry * ce = windows_runtime_exception;

	switch (errorCode) 
	{
		case ERROR_SUCCESS: 
		{
			return;
			//left blank intentionally - no exception should be thrown!
		} break;
		case ERROR_NOACCESS:
		case ERROR_INVALID_ACCESS:
		case ERROR_ACCESS_DENIED:
		case ERROR_INVALID_ACL:
		case ERROR_ACCESS_DISABLED_BY_POLICY:
		{
			ce = windows_access_denied;
		}
		break;
		case ERROR_BAD_PATHNAME:
		case ERROR_REGISTRY_IO_FAILED:
		case ERROR_FILE_NOT_FOUND:
		case ERROR_FILE_CORRUPT:
		case ERROR_FILE_EXISTS:
		case ERROR_FILE_INVALID:
		case ERROR_FILE_READ_ONLY:
		case ERROR_ALREADY_EXISTS:
		{
			ce = windows_invalid_file_exception;
		}
		break;
		case ERROR_KEY_DELETED:
		{
			ce = windows_illegal_operation_exception;
		}
		break;
	}

	if (errorCode != ERROR_SUCCESS) {
		zend_throw_exception_ex(ce, errorCode, get_last_error_str(errorCode));
	}

	ce = NULL;
	efree(ce);

	return;
}
void windows_throw_exception(zend_class_entry * class, char * message)
{
	DWORD hResult = GetLastError();

	//given it doesn't say 'the operation blah balh'
	if (hResult != ERROR_SUCCESS) {
		zend_throw_exception_ex(class, hResult, "%s (%s)", message, get_last_error_str(hResult));
	}
	else {
		zend_throw_exception_ex(class, hResult, "%s", message);
	}
	return;
}