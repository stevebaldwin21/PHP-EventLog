<?php

use Windows\Diagnostics\EventLog;
use Windows\Diagnostics\EventType;

EventLog::setSource("PHP");

if (EventLog::writeEntry("Hello World", EventType::Warning)) {
	return true;
} else {
	return false;
}

?>