#Windows Event Log

This extension is a Windows event log for PHP exposing a super easy API with 2 functions. In combination with the registry extension you can configure your event source soley by PHP. However, it does not allow you to remove event sources. You must do that via the registry.

###Usage
```php

use Windows\Diagnostics\EventLog;
use Windows\Diagnostics\EventType;

//this needs to be configured by you - see 'Registring your event source'
EventLog::setSource("MyPHPLogger");

//start logging...

//types
EventLog::writeEntry("You've been warned...", EventType::Warning);

EventLog::writeEntry("Something went horribly wrong...", EventType::Error);

EventLog::writeEntry("This may require your attention...", EventType::Information);

EventLog::writeEntry("Everything just looks great", EventType::SuccessAudit);

EventLog::writeEntry("Everything doesn't look great", EventType::FailureAudit);

```
###Registering your event source - (manual intevention or the registry module is required)
```php

use Windows\Registry\RegistryHive;
use Windows\Registry\RegistryKey;
use Windows\Registry\RegistryValue;
use Windows\Registry\Exception\InvalidKey;
use Windows\Registry\Exception\AccessDenied;
use Windows\Registry\ValueTypes\Sz;
use Windows\Registry\ValueTypes\Dword;

$registry = RegistryHive::LocalMachine();
$node = $registry->openSubKey("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application");

$key = $node->createSubKey("MyPHPLogger");
$key->setValue("EventMessageFile", new Sz("%SystemRoot%\\System32\\netmsg.dll"));

//required for the availble types
$key->setValue("TypesSupported", new Dword(7));

//done!
```

###Build

To build, you require VC14, php7embed.lib and the php headers defined in this module. This was built with ZTS versioning but no implementation of TSRM required as no globals are used. Maybe for good measure add TSRM local storage to function calls in the future.
