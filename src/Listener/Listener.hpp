#pragma once
#include "Members.hpp"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <thread>
#include <fstream>

#include "Message.hpp"

#include <json/json.hpp>

constexpr size_t HashCount = 10;
constexpr size_t MsgCount = 5;


using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::redirect_error;
using boost::asio::use_awaitable;

class chat_session : public chat_member, public std::enable_shared_from_this<chat_session> {
public:
	chat_session(tcp::socket _socket, chat_room& room):socket(std::move(_socket)),timer(socket.get_executor()),room(room) {
		timer.expires_at(std::chrono::steady_clock::time_point::max());
	}
	void start() {
		room.join(shared_from_this());

		co_spawn(socket.get_executor(), [self = shared_from_this()] {return self->reader(); }, detached);
		co_spawn(socket.get_executor(), [self = shared_from_this()] {return self->writer(); }, detached);
	}
private:
	awaitable<void> reader() {
		try {
		for(std::string read_msg;;){
			auto n = co_await boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_msg, 8192), "\n", use_awaitable);
			auto j = nlohmann::json::parse(read_msg.substr(0,n));	
			if(j.contains("sender") && j.contains("receiver") && j.contains("data") && j.contains("iv") && 
				j.contains("signature") && j.contains("timestamp") && 
				j["iv"].size()==16 && j["signature"].size()==2)
			{
				std::string sender = j["sender"];
				std::string receiver = j["receiver"];
				std::vector<unsigned char> data = j["data"];
				std::array<unsigned char,16> iv = j["iv"];
				std::array<std::string,2> signature = j["signature"];
				time_t timestamp = j["timestamp"];

				Message myMsg{pEC(sender),pEC(receiver),data,iv,signature,timestamp};
				if(myMsg.Verify() && (!room.HasHash(myMsg.GetHash()))){
					room.deliver(std::make_tuple(socket.remote_endpoint(), read_msg.substr(0, n)));
				}
			}
			read_msg.erase(0,n);
		}

		}
		catch (std::exception&) {
			stop();
		}
	}

	awaitable<void> writer() {
		try {
			while (socket.is_open()) {
				if (std::get<1>(_msg).empty()) {
					boost::system::error_code ec;
					co_await timer.async_wait(redirect_error(use_awaitable, ec));
				}
				else {
					co_await boost::asio::async_write(socket, boost::asio::buffer(std::get<1>(_msg)), use_awaitable);
					std::get<1>(_msg).clear();
				}
			}
		}
		catch (std::exception&) {
			stop();
		}
	}

	void stop() {
		room.leave(shared_from_this());
		socket.close();
		timer.cancel();
	}

	void deliver(const msgType& msg) {
		_msg = msg;
		timer.cancel_one();
	}

	tcp::endpoint get_endpoint() {
		return socket.remote_endpoint();
	}

	tcp::socket socket;
	boost::asio::steady_timer timer;
	chat_room& room;
	msgType _msg;
};
