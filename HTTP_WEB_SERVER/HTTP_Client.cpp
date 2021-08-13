#include"HTTP_Client.h"

Client::Client():m_ios() {
	work.reset(new boost::asio::io_service::work(m_ios)); 
	m_worker = std::move(std::thread([this]() {m_ios.run();}));
}

void Client::StartSession(unsigned int id, unsigned int port, const std::string& ip_address, const std::string& resource) {
	if (m_sess.find(id) == m_sess.end()) {
		std::cout << "this id already exists\n";
		return;
	}
	m_sess[id] = std::shared_ptr<Session>(new Session(port,ip_address,resource,m_ios));
	m_sess[id]->Start();
}


void Client::CancelSession(unsigned int id) {
	if (m_sess.find(id) != m_sess.end()) {
		m_sess[id]->Cancel();
	}
	else {
		std::cout << "This session does not exist\n";
	}
}

void Client::finish() {
	work.reset(nullptr);
	m_worker.join();
}


Session::Session(unsigned int port, const std::string& ip,const std::string& res, boost::asio::io_service& s):
	m_port(port),
	m_IPaddress(ip),
	m_resource(res),
	m_canceled(false),
	m_ep(boost::asio::ip::address::from_string(m_IPaddress),m_port),
	m_sock(s),
	m_buffer()
{
	m_sock.open(m_ep.protocol()); // bear in mind that it throws an exception: must catch it...
}

void Session::Start() {
	if (m_canceled.load()==true) {
		finish();
		return;
	}
	m_sock.async_connect(m_ep,std::bind(&Session::SendRequest,this,std::placeholders::_1));
}

void Session::SendRequest(const boost::system::error_code& er){
	
	if (er.value() != 0) {
		std::cout << er.message();
		finish();
		return;
	}
	if (m_canceled.load() == true) {
		finish();
		return;
	}
	
	boost::asio::async_write(m_sock,boost::asio::buffer(m_resource,m_resource.length()),std::bind(&Session::RequestSent,this,std::placeholders::_1,std::placeholders::_2));
}




