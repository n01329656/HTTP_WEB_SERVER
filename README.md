## HTTP_WEB_SERVER
It is a simple connection-oriented HTTP web server that supports the GET method. Basically, this app is divided into three classes:
* Service generates simulates services provided by the Server. In that case, it sends the requested resource using URI to locate the resource.
* Acceptor serves for accepting new connections next spawns a new instance of service in which servers connected the client.
* Server provides simple interface: Start(),Stop(). Start() methods require port number which it is going to listen to for new connections, 
and Stop() method makes all the worker threads complete remaining tasks and exit.
Note: by default E:\\http_pages is a path where the server looks for a specific resource.

You can also find a dummy client class that can request a resource. The purpose of this class is to show how the server works. It is an asynchronous client that can send multiple requests and cancel them at any point.

References:
Radchuk,D.,(2016) Boost.Asio C++ Network Programming Coockbook.PACKT PUBLISHING
