#include <stdio.h>
#include "dl.h"
#include "stats.h"

const char v1version_date[] = __DATE__;
const char v1version_time[] = __TIME__;


Module_Info my_info[] = { {
	"version",
	"changed command hack test for ircd 0.1-beta2",
	"0.1"
} };

int new_m_version(char *origin, char **av, int ac) {
	snumeric_cmd(351, origin, "Module Version Loaded, v%s %s %s",my_info[0].module_version,v1version_date,v1version_time);
	return 0;
}


Functions my_fn_list[] = {
	{ "VERSION",	new_m_version,	1 },
	{ NULL,		NULL,		0 }
};

EventFnList my_event_list[] = {
	{ NULL, 	NULL}
};

EventFnList *__module_get_events() {
	return my_event_list;
};

Module_Info *__module_get_info() {
	return my_info;
}

Functions *__module_get_functions() {
	return my_fn_list;
};

void _init() {
	globops(me.name, "Version Module Loaded");
}
void _fini() {
	globops(me.name, "Version Module Unloaded");
}
