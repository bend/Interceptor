#ifndef HTTP_H_
#define HTTP_H_

class Http {
  public:
  enum ErrorCode : short {
	Ok = 200,
	NotFound = 404,
	BadRequest = 400
  };

static std::string stringValue(ErrorCode error)
{
  switch(error) 
  {
	case Ok:
	  return "OK";
	case NotFound:
	  return "Not Found";
	case BadRequest:
	  return "Bad Request";
	default:
	  return "Unknown";
  }
}

};

#endif // HTTP_H_
