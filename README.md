# Interceptor
[![Build Status](https://travis-ci.org/bend/interceptor.svg?branch=master)](https://travis-ci.org/bend/interceptor)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/f3dec0d2ff6345fa8b11b1d44d3170c9)](https://www.codacy.com/app/bend/Interceptor?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=bend/interceptor&amp;utm_campaign=Badge_Grade)

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
- If-Modified-Since
- Cache-Control
- Reverse Proxy: Forward requests to another backend
- Connectors: Forward matching files to another backend (Can be used for example to forward request for all php files to another server handling php requests.

## TODO

- POST PUT and DELETE requests
- Act as a reverse proxy
- FCGI connector
- Basic Authentication support
- SSL
- Redirections
- ...

## Build & Install

This program need **libz**, **libboost**, **libgamin**, **c++17** compliant compiler
You need cmake to build it.
You can also build it using docker with this command:

   ```
sudo docker run -v $PWD:/code -t interceptor /bin/bash -c "export ENABLE_GZIP=$ENABLE_GZIP; export DEBUG_LOGGING=$DEBUG_LOGGING; export ENABLE_LOCAL_CACHE=$ENABLE_LOCAL_CACHE; cd /code; cmake -DENABLE_GZIP=$ENABLE_GZIP -DDEBUG_LOGGING=$DEBUG_LOGGING . && make && make install && env CTEST_OUTPUT_ON_FAILURE=1 make test"
```

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
		 "locations" : [
			{ 
			  "forbidden" : 403 // default code to return for this directory
			},
			{
			  "/dir" : { // use cache-control for this directory
				"type" : "cache", 
				"expires": "1y"
			  }
			 }, 
			 {
			  "*.php" : { // forward all php files to another connector
				"type": "connector", 
				"name": "php"
			  }
			}
		  ]
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
		  "backend" : "backend_test" // forward all incomming requests for this site to the backend_test
		}
	  ]
	}
  ],
  
   // define backends
   "backends" : [
	{ "name" : "backend_test",
	  "host": "127.0.0.1",
	  "port" : 7000
	},
	{ "name" : "backend_test2",
	  "host": "127.0.0.1",
	  "port" : 7005
	}
  ],

   // define connectors
  "connectors" : [
	{ "name": "php",
	  "type": "fcgi",
	  "host": "localhost",
	  "port" : 1234
	},
	{
	  "name": "nginx",
	  "type": "server",
	  "host": "localhost",
	  "port": 1235
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
