#pragma once
#include <iostream>
#include <memory>
#include <set>
#include <deque>
#include <tuple>
#include <unordered_map>

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
		std::erase_if(hashes,[](const auto& item){
			auto current = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			auto const& [key,value] = item;
			return (current-value)>15;
		});
	}
	bool HasHash(const std::string& hash){
		if(!hashes.contains(hash)){
			hashes[hash] = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			return false;
		}
		return true;
	}
private:
	enum { max_recent_msgs = 100};
	std::set<chat_member_ptr> members;
	std::unordered_map<std::string,time_t> hashes;
};
