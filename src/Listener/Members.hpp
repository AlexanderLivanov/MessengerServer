#pragma once
#include <iostream>
#include <memory>
#include <set>
#include <deque>
#include <tuple>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

using boost::asio::ip::tcp;

typedef std::tuple<tcp::endpoint,std::string> msgType;

class chat_member{
public:
	virtual ~chat_member(){}
	virtual void deliver(const msgType& msg) = 0;
	virtual tcp::endpoint get_endpoint() = 0;
};

typedef std::shared_ptr<chat_member> chat_member_ptr;

class chat_room {
public:
	void join(chat_member_ptr member) {
		members.insert(member);
	}
	void leave(chat_member_ptr member) {
		members.erase(member);
	}
	void deliver(const msgType& msg) {
		for (auto member : members) {
			if (member->get_endpoint() != std::get<0>(msg)) {
				member->deliver(msg);
			}
		}
	}
private:
	enum { max_recent_msgs = 100};
	std::set<chat_member_ptr> members;
};
