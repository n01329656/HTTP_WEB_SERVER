#include"HTTP_Server.h"
using namespace boost;


	Service::Service(std::shared_ptr<boost::asio::ip::tcp::socket> sock):
		m_sock(sock),
		m_request(4096),
		m_response_status_code(200), 
		m_resource_size_bytes(0)
	{};
	void Service::StartHandling() {
		asio::async_read_until(*m_sock.get(), m_request, "\r\n", 
			std::bind(&Service::on_request_line_received,this,std::placeholders::_1,std::placeholders::_2));
			
	}
	void Service::on_request_line_received(const boost::system::error_code& ec,std::size_t bytes_transferred)
	{
		if (ec.value() != 0) {
			std::cout << "Error occured! Error code = "
				<< ec.value()
				<< ". Message: " << ec.message();
			if (ec == asio::error::not_found) {
				// No delimiter has been found in the
				// request message.
				m_response_status_code = 413;
				send_response();
				return;
			}
			else {
				// In case of any other error –
				// close the socket and clean up.
				on_finish();
				return;
			}
		}

		std::string request_line;
		std::istream request_stream(&m_request);
		std::getline(request_stream, request_line, '\r');
		request_stream.get();
		std::string request_method;
		std::istringstream request_line_stream(request_line);
		request_line_stream >> request_method;
		if (request_method.compare("GET") != 0) {
		
			m_response_status_code = 501;
			send_response();
			return;
		}
		request_line_stream >> m_requested_resource;
		std::string request_http_version;
		request_line_stream >> request_http_version;
		if (request_http_version.compare("HTTP/1.1") != 0) {
			// Unsupported HTTP version or bad request.
			m_response_status_code = 505;
			send_response();
			return;
		}
		// At this point the request line is successfully
		// received and parsed. Now read the request headers.
		asio::async_read_until(*m_sock.get(), m_request, "\r\n\r\n",
			std::bind(&Service::on_headers_received,this,std::placeholders::_1,std::placeholders::_2));
			
		return;
	}
	void Service::on_headers_received(const boost::system::error_code& ec,std::size_t bytes_transferred)
	{
		if (ec.value() != 0) {
			std::cout << "Error occured! Error code = "
				<< ec.value()
				<< ". Message: " << ec.message();
			if (ec == asio::error::not_found) {
				// No delimiter has been fonud in the
				// request message.
				m_response_status_code = 413;
				send_response();
				return;
			}
			else {
				// In case of any other error - close the
				// socket and clean up.
				on_finish();
				return;
			}
		}
		// Parse and store headers.
		std::istream request_stream(&m_request);
		std::string header_name, header_value;
		while (request_stream) {
			std::getline(request_stream, header_name, ':');
			if (!request_stream.eof()) {
				std::getline(request_stream,header_value,'\r');
				// Remove symbol \n from the stream.
				request_stream.get();
				m_request_headers[header_name] =header_value;
			}
		}
		// Now we have all we need to process the request.
		process_request();
		send_response();
		return;
	}
	void Service::process_request() {
		// Read file.
		std::string resource_file_path =std::string("E:\\http_pages") + m_requested_resource;

		if (! boost::filesystem::exists(resource_file_path)) {
			// Resource not found.
			m_response_status_code = 404;
			return;
		}
		std::ifstream resource_fstream(resource_file_path,std::ifstream::binary);
		
		if (!resource_fstream.is_open()) {
			// Could not open file. 
			// Something bad has happened.
			m_response_status_code = 500;
				return;
		}
		// Find out file size.
		resource_fstream.seekg(0, std::ifstream::end);
		m_resource_size_bytes = resource_fstream.tellg();
		m_resource_buffer.reset(new char[m_resource_size_bytes]);
		resource_fstream.seekg(std::ifstream::beg);
		resource_fstream.read(m_resource_buffer.get(),m_resource_size_bytes);
		m_response_headers += std::string("content-length") +": " +std::to_string(m_resource_size_bytes) + "\r\n";
	}
	void  Service::send_response() {
		m_sock->shutdown(asio::ip::tcp::socket::shutdown_receive);
		auto status_line =http_status_table.at(m_response_status_code);
		m_response_status_line = std::string("HTTP/1.1 ") +status_line +"\r\n";
		m_response_headers += "\r\n";
		std::vector<asio::const_buffer> response_buffers;
		response_buffers.push_back(asio::buffer(m_response_status_line));
		if (m_response_headers.length() > 0) {
				response_buffers.push_back(asio::buffer(m_response_headers));
			}

		if (m_resource_size_bytes > 0) {
			response_buffers.push_back(asio::buffer(m_resource_buffer.get(),m_resource_size_bytes));
		}
		// Initiate asynchronous write operation.
		asio::async_write(*m_sock.get(), response_buffers,std::bind(&Service::on_response_sent,this,std::placeholders::_1,std::placeholders::_2));
			
	}
	void  Service::on_response_sent(const boost::system::error_code& ec,std::size_t bytes_transferred)
	{
		if (ec.value() != 0) {
			std::cout << "Error occured! Error code = "
				<< ec.value()
				<< ". Message: " << ec.message();
		}
		m_sock->shutdown(asio::ip::tcp::socket::shutdown_both);
		on_finish();
	}
	void  Service::on_finish() {
		delete this;
	}
  const std::map<unsigned int, std::string> Service::http_status_table = {
 { 200, "200 OK" },
 { 404, "404 Not Found" },
 { 413, "413 Request Entity Too Large" },
 { 500, "500 Server Error" },
 { 501, "501 Not Implemented" },
 { 505, "505 HTTP Version Not Supported" }
	};


	Acceptor::Acceptor(asio::io_service& ios, unsigned short port_num) :
		m_ios(ios),
		m_acceptor(m_ios,
			asio::ip::tcp::endpoint(
				asio::ip::address_v4::any(),
				port_num)),
		m_isStopped(false)
	{}
	void Acceptor::Start() {
		m_acceptor.listen();
		InitAccept();
	}
	void Acceptor::Stop() {
		m_isStopped.store(true);
	}
	void Acceptor::InitAccept() {
			std::shared_ptr<asio::ip::tcp::socket>
			sock(new asio::ip::tcp::socket(m_ios));
		m_acceptor.async_accept(*sock.get(),
			[this, sock](const boost::system::error_code& error)
			{
				onAccept(error, sock);
			});
	}
	void Acceptor::onAccept(const boost::system::error_code& ec,std::shared_ptr<asio::ip::tcp::socket> sock)
	{
		if (ec.value() == 0) {
			(new Service(sock))->StartHandling();
		}
		else {
			std::cout << "Error occured! Error code = "
				<< ec.value()
				<< ". Message: " << ec.message();
		}
		// Init next async accept operation if
		// acceptor has not been stopped yet.
		if (!m_isStopped.load()) {
			this->InitAccept();
		}
		else {
			// Stop accepting incoming connections
			// and free allocated resources.
			m_acceptor.close();
		}
	}


	Server::Server() {
		m_work.reset(new boost::asio::io_service::work(m_ios));
	}
	void Server::Start(unsigned short port_num,unsigned int thread_pool_size) {

		assert(thread_pool_size > 0);
		// Create and start Acceptor.
		acc.reset(new Acceptor(m_ios, port_num));
		acc->Start();
		// Create specified number of threads and 
		// add them to the pool.
		for (unsigned int i = 0; i < thread_pool_size; i++) {
			std::unique_ptr<std::thread> th(
				new std::thread([this]()
					{
						m_ios.run();
					}));
			m_thread_pool.push_back(std::move(th));
		}
	}
	void Server::Stop() {
		acc->Stop();
		m_ios.stop();
		for (auto& th : m_thread_pool) {
			th->join();
		}
	}
	

