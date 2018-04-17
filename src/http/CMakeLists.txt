
MACRO (FILE_TO_STRING infile outfile var) 
	MESSAGE(${CMAKE_CURRENT_SOURCE_DIR})
	ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${outfile} 
		COMMAND ${CMAKE_COMMAND} 
		ARGS	-Dinfile:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/${infile} 
		-Doutfile:FILEPATH=${CMAKE_CURRENT_BINARY_DIR}/${outfile} 
		-Dvar:STRING=${var} 
		-P ${PROJECT_SOURCE_DIR}/cmake/filetostring.cmake 
		MAIN_DEPENDENCY 
		${CMAKE_CURRENT_SOURCE_DIR}/${infile}
		COMMENT "Generating file " ${outfile}
		)
ENDMACRO(FILE_TO_STRING)

FILE_TO_STRING(html/header.html.in HtmlHeader.cpp HtmlHeader)
FILE_TO_STRING(html/footer.html.in HtmlFooter.cpp HtmlFooter)


set(libhttp_src
  Headers.cpp
  Request.cpp
  Reply.cpp
  CommonReply.cpp
  GetReply.cpp
  PostReply.cpp
  HeadReply.cpp
  ErrorReply.cpp
  AuthenticationReply.cpp
  RedirectReply.cpp
	DirectoryListingReply.cpp
  Http.cpp
  HTTP11SessionHandler.cpp
  Encoder.cpp
	HtmlHeader.cpp
	HtmlFooter.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_http STATIC ${libhttp_src})

