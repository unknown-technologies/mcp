#include <emulight/tso.h>

TENV* env;

int start(TENV* tenv)
{
	env = tenv;
	return TMain(env->argc, env->argv);
}
