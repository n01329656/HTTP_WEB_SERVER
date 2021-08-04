## HTTP_WEB_SERVER
It is a simple connection-oriented HTTP web server that supports the GET method. Basically, this app is divided into three classes:
* Service
* Acceptor
* Server
Service call simulates services provided by the Server. In that case, it sends the requested resource using URI to locate the resource.
Acceptor servers for accepting new connections next spawns a new instance of service in which servers connected the client.
Server class provides simple interface: Start(),Stop(). Start() methods require port number which it is going to listen to for new connections, 
and Stop() method makes all the worker threads complete remaining tasks and exit.

References:
Radchuk,D.,(2016) Boost.Asio C++ Network Programming Coockbook.PACKT PUBLISHING
