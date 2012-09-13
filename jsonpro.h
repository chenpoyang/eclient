/* json数据形式的自定义协议 */
#ifndef _JSONPRO_H_
#define _JSONPRO_H_

#define EME_JSON_SPLIT "\r\n"

/* login */
#define EME_LOGIN_ACTION_KEY "action"
#define EME_LOGIN_VALUE_KEY  "login"
#define EME_LOGIN_USER_KEY   "userid"
#define EME_LOGIN_PWD_KEY    "password"
#define EME_LOGINRES_ACTION_KEY "action"
#define EME_LOGINRES_VALUE_KEY  "login"
#define EME_LOGINRES_STAT_KEY   "stat"

/* register */
#define EME_REGISTER_ACTION_KEY "action"
#define EME_REGISTER_VALUE_KEY  "reg"
#define EME_REGISTER_USER_KEY   "userid"
#define EME_REGISTER_PWD_KEY    "password"
#define EME_REGISTER_REPWD_KEY  "repassword"
#define EME_REGISTERRES_ACTION_KEY "action"
#define EME_REGISTERRES_VALUE_KEY  "reg"
#define EME_REGISTERRES_STAT_KEY   "stat"

/* send msg */
#define EME_SENDMSG_ACTION_KEY "action"
#define EME_SENDMSG_VALUE_KEY  "sendmsg"
#define EME_SENDMSG_MSG_KEY    "msg"
#define EME_SENDMSG_TO_KEY     "to"
#define EME_SENDMSG_FROM_KEY   "from"
#define EME_SENDMSGRES_ACTION_KEY "action"
#define EME_SENDMSGRES_VALUE_KEY  "sendmsg"
#define EME_SENDMSGRES_STAT_KEY   "stat"

/* received msg */
#define EME_RECVMSG_ACTION_KEY  "action"
#define EME_RECVMSG_VALUE_KEY   "sendmsg"
#define EME_RECVMSG_MSG_KEY     "msg"
#define EME_RECVMSG_TO_KEY      "to"
#define EME_RECVMSG_FROM_KEY    "from"

#endif /* _JSONPRO_H_ */
