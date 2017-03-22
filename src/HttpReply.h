#ifndef HTTP_REPLY_H__
#define HTTP_REPLY_H__

#include "Defs.h"

class HttpReply {
  
  public:
	HttpReply(HttpRequestPtr request);

	void process();

};

#endif // HTTP_REPLY_H__
