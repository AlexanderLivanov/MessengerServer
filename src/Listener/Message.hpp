#pragma once
#include "../Crypto/EC/EC.hpp"
#include "../Crypto/HASH/SHA256.hpp"
#include "../Crypto/ChaCha20/ChaCha20.hpp"


struct Message final{
    pEC sender;
    pEC receiver;

    std::vector<unsigned char> data;

    std::array<unsigned char, 12> iv; //For ChaCha20
    std::array<std::string,2> signature; // 0 - R, 1 - S numbers in ECDSA

    time_t timestamp;

    const std::string GetHash(){
        std::vector<unsigned char> _data = data;
        _data.insert(_data.end(),iv.begin(),iv.end());
        auto key = receiver.GetPkey();
        _data.insert(_data.end(),key.begin(),key.end());
        auto _timestamp = std::to_string(timestamp);
        _data.insert(_data.end(),_timestamp.begin(),_timestamp.end());
        return sha256(_data);
    }

    const bool Verify(const bool& checkTime = true){ //digital signature will be incorrect if we change pEC sender
        auto res = sender.Verify(GetHash(),signature);
        if(checkTime){
            const auto current = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            return res && (std::abs(current - timestamp) <= 15);
        }
        return res;
    }
    
    const std::vector<unsigned char> DecryptViaSender(sEC _sender){
        auto key = _sender.Exchange(receiver);
        return chacha20_dec(data,key,iv);
    }
    const std::vector<unsigned char> DecrypViaReceiver(sEC _receiver){
        auto key = _receiver.Exchange(sender);
        return chacha20_dec(data,key,iv);
    }
};
