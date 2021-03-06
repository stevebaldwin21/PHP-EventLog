#include "eventlog.h"
#include "exceptions.h"
#include "zend_interfaces.h"
#include "zend_vm.h"
#include "zend_execute.h"

/*
* 
* Notes on this extension;
* 
* setSource does NOT use RegisterEventSource instead stores an internal variable statically, which is retrived upon using writeEntry.
* This is to prevent the overuse of the event log. Ideally, an application should maintain 1 event log only
* This extension does not register event sources, you can do this via the registry.c/registry.h files if desired.
* Neither class is instantiationable, both are explicit abstract class with the ZEND_ACC_FINAL|ZEND_ACC_EXPLICIT_ABS. flag.
* Additionally, this extension is stateless that meaning, no handles are stored. Each time you call writeEntry to handle is retrived to access
* and directly closed.
* 
* see https://wiki.php.net/rfc/fast_zpp
* 
* @author Steve Baldwin <stevebaldwin21@googlemail.com>
* Report bugs to the above email address
*/

/* {{{ proto EventLog::writeEvent(string message, EventLog::type eventtype) */
PHP_METHOD(EventLog, writeEntry)
{
	int  * sourceLength;
	char * message = (char*)emalloc(sizeof(char*));
	int  * messageLength;
	int eventType = EVENTLOG_INFORMATION_TYPE; //default if none is provided
	const char *errarg[10];
	int response;
	char * source; 
	HANDLE sourceHandle = NULL;
	DWORD dwEventDataSize = 0;
	zval * this_source;

	this_source = zend_read_static_property(eventlog_log_class, "source", strlen("source"), 1);

	if (this_source == NULL || Z_TYPE_P(this_source) == IS_NULL)
	{
		windows_throw_exception(windows_invalid_argument_exception, "Please provide a source before attempting to write");
		this_source = NULL;
		efree(this_source);
		RETURN_FALSE;
	}

	source = Z_STRVAL_P(this_source);
	this_source = NULL;
	efree(this_source);

	ZEND_PARSE_PARAMETERS_START(1, 2);
		Z_PARAM_STRING_EX(message, messageLength, 1, 0)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(eventType)
	ZEND_PARSE_PARAMETERS_END();

	//validation!
	if (zend_ht_val_exists_long(&event_class->constants_table, eventType) == false) 
	{
		windows_throw_exception(windows_invalid_argument_exception, "Please provide a valid event type using the EventType class");
		RETURN_FALSE;
	}

	if (message[0] == '\0') {
		windows_throw_exception(windows_invalid_argument_exception, "Please provide a valid string message");
		RETURN_FALSE;
	}

	//The provider name!
	sourceHandle = RegisterEventSource(NULL, source);

	if (sourceHandle == NULL) {
		//handle will 0x000000 no response
		windows_throw_exception_hres(GetLastError());
	}

	//the format we only care for the first string!
	errarg[0] = message;
	errarg[1] = NULL;
	errarg[2] = NULL;
	errarg[3] = NULL;
	errarg[4] = NULL;
	errarg[5] = NULL;
	errarg[6] = NULL;
	errarg[7] = NULL;
	errarg[8] = NULL;

	//3299 a generic message in netmsg.dll - use this with 9 char * array for simple message
	response = ReportEvent(sourceHandle, eventType, NULL, 3299, NULL, 9, 0, &errarg, NULL);

	if (response == ERROR_INVALID_FUNCTION) 
	{
		//this is scenario if the user has not registered their event source
		//but we shouldn't throw an exception because the log will still be logged. It just will moan about the unregistered source message/resource
		php_error(E_WARNING, "Invalid event source '%s' - has this been registered?", source);
	}
	else 
	{
		message = NULL;
		efree(message);
		CloseEventLog(sourceHandle);
		windows_throw_exception_hres(response);
		RETURN_FALSE;
	}

	message = NULL;
	efree(message);

	CloseEventLog(sourceHandle);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto EventLog::setSource(string source) */
PHP_METHOD(EventLog, setSource)
{
	char * source = (char*)emalloc(sizeof(char*));
	int sourceLength;

	ZEND_PARSE_PARAMETERS_START(1, 2);
		Z_PARAM_STRING_EX(source, sourceLength, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	if (source[0] == '\0' || sourceLength == 0) {
		windows_throw_exception(windows_invalid_argument_exception, "Please provide a valid source");
		RETURN_FALSE;
	}

	zend_declare_property_string(eventlog_log_class, "source", strlen("source"), source, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC);

	source = NULL;
	efree(source);
}
/* }}} */

/* {{{ proto EventLog::getSource(void) */
PHP_METHOD(EventLog, getSource) {
	GET_STATIC_PROPERTY(eventlog_log_class, "source");
}
/* }}} */

/*
* Argument information and functions!
*/
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_el_write_source, 0, 2, _IS_BOOL, NULL, 0)
ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, eventType, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_el_set_source, 0, 1, IS_NULL, 0, 0)
ZEND_ARG_TYPE_INFO(0, source, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ai_el_get_source, 0, 1, IS_STRING, 0, 0)
ZEND_END_ARG_INFO()

zend_function_entry eventlog_functions[] = {
	PHP_ME(EventLog, writeEntry, ai_el_write_source, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(EventLog, setSource, ai_el_set_source, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(EventLog, getSource, ai_el_get_source, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

PHP_MINIT_FUNCTION(windows_event_log)
{
	INIT_CLASS_ENTRY(ce, EVENTLOG_NS("EventLog"), eventlog_functions);
	eventlog_log_class = zend_register_internal_class(&ce TSRMLS_CC);
	eventlog_log_class->ce_flags |= ZEND_ACC_FINAL;
	eventlog_log_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;

	INIT_CLASS_ENTRY(ce, EVENTLOG_NS("EventType"), NULL);
	event_class = zend_register_internal_class(&ce TSRMLS_CC);
	event_class->ce_flags |= ZEND_ACC_FINAL;
	event_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;

	zend_declare_class_constant_long(event_class, "Error", strlen("Error"), EVENTLOG_ERROR_TYPE);
	zend_declare_class_constant_long(event_class, "Information", strlen("Information"), EVENTLOG_INFORMATION_TYPE);
	zend_declare_class_constant_long(event_class, "SuccessAudit", strlen("SuccessAudit"), EVENTLOG_AUDIT_SUCCESS);
	zend_declare_class_constant_long(event_class, "FailureAudit", strlen("FailureAudit"), EVENTLOG_AUDIT_FAILURE);
	zend_declare_class_constant_long(event_class, "Warning", strlen("Warning"), EVENTLOG_WARNING_TYPE);

	SUCCESS;
}

PHP_MINFO_FUNCTION(windows_event_log)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Windows Event Log", "enabled");
	php_info_print_table_end();
}

zend_module_entry windows_event_log_module_entry = {
	STANDARD_MODULE_HEADER,
	"windows_event_log",
	NULL,
	PHP_MINIT(windows_event_log), NULL, NULL, NULL, NULL,
	NO_VERSION_YET, STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(windows_event_log)
