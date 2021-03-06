#include "SocketServer.h"


asio::io_service* SocketServer::io_service_ = new asio::io_service;

//TcpConnection::~TcpConnection()
//{
//	std::cout << "delete";
//	delete_from_parent();
//}

TcpConnection::pointer TcpConnection::create(asio::io_service& io_service, SocketServer* parent)
{
	return pointer(new TcpConnection(io_service, parent));
}

tcp::socket& TcpConnection::socket()
{
	return socket_;
}

void TcpConnection::start()
{
	asio::async_read(socket_,
	                 asio::buffer(read_msg_.data(), socket_message::header_length),
	                 std::bind(&TcpConnection::handle_read_header, this,
	                           std::placeholders::_1));
}

void TcpConnection::write_data(std::string s)
{
	socket_message msg;
	if (s.size() == 0)
	{
		s = std::string("\0");
		msg.body_length(1);
	}
	else
		msg.body_length(s.size());
	memcpy(msg.body(), &s[0u], msg.body_length());
	msg.encode_header();
	asio::write(socket_,
	            asio::buffer(msg.data(), msg.length()));
}

std::string TcpConnection::read_data()
{
	std::unique_lock<std::mutex> lk{mut_};
	while (read_msg_deque_.empty())
		data_cond_.wait(lk);
	auto read_msg = read_msg_deque_.front();
	read_msg_deque_.pop_front();
	lk.unlock();
	auto ret = std::string(read_msg.body(), read_msg.body_length());
	return ret;
}

void TcpConnection::do_close()
{
	socket_.close();
	delete_from_parent();
}

void TcpConnection::handle_read_header(const asio::error_code& error)
{
	if (!error && read_msg_.decode_header())
	{
		std::cout << "here\n";
		asio::async_read(socket_,
		                 asio::buffer(read_msg_.body(), read_msg_.body_length()),
		                 std::bind(&TcpConnection::handle_read_body, this,
		                           std::placeholders::_1));
	}
	else
	{
		do_close();
	}
}

void TcpConnection::handle_read_body(const asio::error_code& error)
{
	if (!error)
	{
		std::lock_guard<std::mutex> lk{mut_};
		read_msg_deque_.push_back(read_msg_);
		data_cond_.notify_one();
		asio::async_read(socket_,
		                 asio::buffer(read_msg_.data(), socket_message::header_length),
		                 std::bind(&TcpConnection::handle_read_header, this,
		                           std::placeholders::_1));
	}
	else
	{
		do_close();
	}
}

TcpConnection::TcpConnection(asio::io_service& io_service, SocketServer* parent):
	socket_(io_service), parent(parent)
{
	std::cout << "new tcp" << std::endl;
}


void TcpConnection::delete_from_parent()
{
	if (parent)
		shared_from_this()->parent->remove_connection(shared_from_this());
	parent = nullptr;
}


SocketServer* SocketServer::create(int port)
{
	auto s = new SocketServer(port);
	s->thread_ = new std::thread(
		std::bind(static_cast<std::size_t(asio::io_service::*)()>(&asio::io_service::run),
		          io_service_));
	return s;
}

void SocketServer::button_start()
{
	using namespace std; // For sprintf and memcpy.
	char total[4 + 1] = "";
	sprintf(total, "%4d", static_cast<int>(connections_.size()));

	for (auto i = 0; i < connections_.size(); i++)
		connections_[i]->write_data("PLAYER" + std::string(total) + std::to_string(i + 1));
	connection_num_ = connections_.size();
	this->button_thread_ = new std::thread(std::bind(&SocketServer::loop_process, this));
	button_thread_->detach();
}

SocketServer::SocketServer(int port):
	acceptor_(*io_service_, tcp::endpoint(tcp::v4(), port))
{
	start_accept();
}

void SocketServer::loop_process()
{
	while (true)
	{
//		if (connections_.size() != connection_num)
//			throw std::exception{"lost connection"};
		std::vector<std::string> ret;
		for (auto r : connections_)
			ret.push_back(r->read_data());
		auto game_msg = GameMessageWrap::combine_message(ret);
		for (auto r : connections_)
			r->write_data(game_msg);
	}
}

std::vector<TcpConnection::pointer> SocketServer::get_connection() const
{
	return connections_;
}

void SocketServer::remove_connection(TcpConnection::pointer p)
{
	//		connections_.erase(std::remove(connections_.begin(), connections_.end(), p), connections_.end());
	auto position = std::find(connections_.begin(), connections_.end(), p);

	if (position == connections_.end())
		std::cout << "delete not succ\n";

	else
		connections_.erase(position);
	std::cout << "delete succ\n";
}


void SocketServer::start_accept()
{
	TcpConnection::pointer new_connection =
		TcpConnection::create(acceptor_.get_io_service(), this);

	acceptor_.async_accept(new_connection->socket(),
	                       std::bind(&SocketServer::handle_accept, this, new_connection,
	                                 std::placeholders::_1));
	std::cout << "start accept " << std::endl;
}

void SocketServer::handle_accept(TcpConnection::pointer new_connection, const asio::error_code& error)
{
	std::cout << "handle_accept\n";
	if (!error)
	{
		connections_.push_back(new_connection);
		std::cout << new_connection->socket().remote_endpoint().address()
			<< ":" << new_connection->socket().remote_endpoint().port() << std::endl;
		new_connection->start();
	}
	start_accept();
	//			std::cout << "handle accept\n";
}
