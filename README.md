# Interceptor
[![Build Status](https://travis-ci.org/bend/interceptor.svg?branch=master)](https://travis-ci.org/bend/interceptor)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/f3dec0d2ff6345fa8b11b1d44d3170c9)](https://www.codacy.com/app/bend/Interceptor?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=bend/Interceptor&amp;utm_campaign=Badge_Grade)

A fast Web server written in C++ 

## Actual Support (will be updated while the project evolves)

- GET request
- HEAD request
- Partial Request
- Chunking
- Gzip Compression
- Etag, LastModified date
- Server Timeout
- Virtual servers
- Custom Error pages
- Locations access restriction
- Local cache 

## Build & Install

This program need libz, libboost, **c++14** compliant compiler

### Compilation Flags 
  
  ```
  ENABLE_GZIP=[on|off] // whether you want to enable gzip support for compression, required libz
  DEBUG_LOGGING=[on|off] // whether you want verbose logging or not
  ENABLE_LOCAL_CACHE=[on|off] // whether you want to enable local caching or not
```
To compile it just run 

```
cmake . && make
```

## Configuration 

You can find a configuration example in the config/ directory. Be carefull that comments are *not* supported by the json standard.

```
{
  "global" :  {
	"server-timeout" : 200, //timeout in seconds
	"client-timeout" : 200, //timeout in seconds
	"nb-threads" : 10, // number of threads to use by default it will use the number of cores
	"max-cache-size" : 100, // Size of the local cache to keep (in MB)
	"max-request-size": 4,  // Maximum allowed size for incomming requests
	"max-in-mem-request" : 2,	// Maximum size of a request on ram, after that the request will be swapped
	"error-pages" : [
	  { "404" : "/home/ben/projects/interceptor/config/404.html" }, // displayed on error 404
	  { "400" : "/home/ben/projects/interceptor/config/400.html" }  // displayed on error 400, if no page is specified or page is
                                                                        // not found, a default page will be returned
	]
  },
  "servers": [          
	{                           // first server, each server listens on a different port and can host multiple sites.
	  "listen-host": "0.0.0.0", // binding host
	  "listen-port": 8000,      // binding port
	  "server-timeout": 250,    // send timeout, overwrites global value
	  "client-timeout": 250,    // read timeout, overwrites global value
	  "sites": [
		{
		  "gzip": "css,html,js,png", // enable gzip compression for this site, only for those file type. "all" can be used for all filetypes
		  "try-files" : [            // list of default indexes to look for
			"index.php",
			"index.html"
		  ],
		  "host": "0xff.xyz",       // host of the site
		  "docroot": "/usr/share/nginx/homepage/",    // root of the site
		  "error-pages" : [         // error pages, overwrites default values
			{ "404" : "404.html" },
			{ "400" : "400.html" }
		  ],
		  "locations" : [			// Default code to return for a specific location
			{ "/forbidden" : 403 } // The string is a regex using the ECMA grammar
		  }
		}
	  ]
	},
	{                           // second server
	  "listen-host": "0.0.0.0",
	  "listen-port": 8002,

	  "sites": [
		{
		  "gzip": "all",
		  "try-files" : [
			"index.php",
			"index.html"
		  ],
		  "host": "0xff.xyz",
		  "docroot": "/home/ben/docker-data/cv-data/"
		}
	  ]
	}


  ]
}
````
## Running

The most simple way to run the progam is

````
./interceptor --config config.json
````

You can find more parameters with the --help flag
