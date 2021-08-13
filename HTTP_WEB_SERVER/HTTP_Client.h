#pragma once
#include<boost/asio.hpp>
#include<string>
#include<memory>
#include<map>
#include<algorithm>
#include<iostream>

struct Session;

class Client {

	Client();
	void StartSession(unsigned int id, unsigned int port, const std::string& ip_address, const std::string& resource);
	void CancelSession(unsigned int id);
	void finish();

private:
	boost::asio::io_service m_ios;
	std::unique_ptr<boost::asio::io_service::work>work;
	std::thread m_worker;
	std::map<unsigned int, std::shared_ptr<Session>> m_sess;
};

struct Session {
private:
	unsigned int m_port;
	std::string m_IPaddress; 
	const std::string m_resource;
	std::atomic<bool>m_canceled;
	boost::asio::ip::tcp::endpoint m_ep;
	boost::asio::ip::tcp::socket m_sock;
	boost::asio::streambuf m_buffer;
	void finish();

public:
	Session(unsigned int port,const std::string& ip,const std::string& res, boost::asio::io_service& s);
	void Start();
	void SendRequest(const boost::system::error_code& er);
	void RequestSent(const boost::system::error_code& er,std::size_t bytes);
	void ResponseReceived(const boost::system::error_code& er, std::size_t bytes);
	void Cancel();
};