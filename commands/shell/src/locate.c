#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/ds.h>
#include <emulight/tso.h>

static char prefix[32];
static char dsnbuf[32];
static BOOL found;
static BOOL ambiguous;

BOOL startswith(const char* a, const char* b)
{
	while(*a == *b) {
		if(!*a) {
			return TRUE;
		}
		a++;
		b++;
	}

	return !*b;
}

static const char* lastlevel(const char* s)
{
	const char* end;
	for(end = s; *end; end++);

	while(end > s) {
		if(*end != '.') {
			end--;
		} else {
			break;
		}
	}

	return end + 1;
}

static BOOL TLocateCallback(const char* dsn, void* user)
{
	const char* name = lastlevel(dsn);
	const char* cmdname = (const char*) user;

	if(!strcmp(name, cmdname)) {
		found = TRUE;
		strcpy(dsnbuf, dsn);
		return FALSE;
	} else {
		return TRUE;
	}
}

static BOOL TLocateShortCallback(const char* dsn, void* user)
{
	const char* name = lastlevel(dsn);
	const char* cmdname = (const char*) user;

	if(startswith(name, cmdname)) {
		if(found) {
			ambiguous = TRUE;
			return FALSE;
		} else {
			found = TRUE;
			ambiguous = FALSE;
			strcpy(dsnbuf, dsn);
		}
	}

	return TRUE;
}

char* TLocate(TENV* env)
{
	bzero(dsnbuf, sizeof(dsnbuf));

	if(env->argc == 0) {
		return NULL;
	}

	const char* name = env->argv->value;

	if(strlen(name) > 8) {
		TPrint("INVALID COMMAND\n");
		return NULL;
	}

	sprintf(prefix, "SYS1.COMMAND.%s", name);

	found = FALSE;
	FSList(prefix, TLocateCallback, (void*) name);

	if(found) {
		return dsnbuf;
	}

	/* no command found, but now check if it's unique */
	found = FALSE;
	FSList(prefix, TLocateShortCallback, (void*) name);

	if(found) {
		if(ambiguous) {
			TPrint("AMBIGUOUS COMMAND\n");
			return NULL;
		} else {
			return dsnbuf;
		}
	} else {
		TPrint("NOT FOUND\n");
		return NULL;
	}
}
