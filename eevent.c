#include "eevent.h"
#include "elog.h"

void e_login_result(int result)
{
    e_debug("e_login_result", "login result:[%d]", result);
}

void e_register_result(int result)
{
    e_debug("e_register_result", "register result:[%d]", result);
}
