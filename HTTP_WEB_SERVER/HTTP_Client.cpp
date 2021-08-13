#include"HTTP_Client.h"





Client::Client():m_ios() {
	work.reset(new boost::asio::io_service::work(m_ios)); 
	m_worker = std::move(std::thread([this]() {m_ios.run();}));
}

void Client::StartSession(unsigned int id, unsigned int port, const std::string& ip_address, const std::string& resource) {
	auto it = m_sess.find(id);
	if (it != m_sess.end()) {
		if (it->second != nullptr) {
			std::cout << "this id already exists\n";
			return;
		}
	}
	m_sess[id] = new Session(port,ip_address,resource,m_ios);
	m_sess[id]->Start();
}

void Client::CancelSession(unsigned int id) {
	if (m_sess.find(id) != m_sess.end()) {
		m_sess[id]->Cancel();
		m_sess.erase(id);
	}
	else {
		std::cout << "This session does not exist\n";
	}
}

void Client::finish() {
	work.reset(nullptr);
	m_worker.join();
}


Client::Session::Session(unsigned int port, const std::string& ip, const std::string& res, boost::asio::io_service& s) :
	m_port(port),
	m_request(),
	m_resource(res),
	m_canceled(false),
	m_ep(boost::asio::ip::address::from_string(ip),m_port),
	m_sock(s),
	m_buffer()
{
	m_sock.open(m_ep.protocol()); // bear in mind that it throws an exception: must catch it...
}

void Client::Session::Start() {
	if (m_canceled.load()==true) {
		finish();
		return;
	}
	m_sock.async_connect(m_ep,std::bind(&Session::SendRequest,this,std::placeholders::_1));
}

void Client::Session::SendRequest(const boost::system::error_code& er){
	
	if (er.value() != 0) {
		std::cout << er.message();
		finish();
		return;
	}
	
	 m_request = "GET " + m_resource + " HTTP/1.1"+ " \r\n"
		 + " \r\n\r\n"; // assume that we have no headers

	if (m_canceled.load() == true) {
		finish();
		return;
	}
	
	boost::asio::async_write(m_sock,boost::asio::buffer(m_request,m_request.length()),std::bind(&Session::RequestSent,this,std::placeholders::_1,std::placeholders::_2));
}

void Client::Session::RequestSent(const boost::system::error_code& er, std::size_t bytes) {
	
	if (er.value() != 0) {
		std::cout << er.message();
		finish();
		return;
	}

	m_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
	if (m_canceled.load() == true) {
		finish();
		return;
	}
	boost::asio::async_read_until(m_sock,m_buffer,EOF,std::bind(&Session::ResponseReceived,this,std::placeholders::_1,std::placeholders::_2));
}

void Client::Session::ResponseReceived(const boost::system::error_code& er,std::size_t bytes) {
	
	if (er.value() != 0 && m_buffer.size()==0 ) {
		std::cout <<"An error occured: " <<er.value()<<" " << er.message()<<"\n";
		finish();
		return;
	}

	m_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
	if (m_canceled.load() == true) {
		finish();
		return;
	}

	std::istream input(&m_buffer);
	std::cout << "Content of response:\n";
	std::cout << input.rdbuf(); // just dumping the whole response from the server to see if it works properly
	std::cout << "\n\n";
	finish();
}

inline void Client::Session::Cancel() {
	m_canceled.store(true); // setting cancel flag to true 
}

void Client::Session::finish() {
	if (m_sock.is_open()) {
		m_sock.cancel();
	}
	delete this; 
}
	
