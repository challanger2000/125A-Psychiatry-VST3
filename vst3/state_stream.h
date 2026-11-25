#pragma once
#include "pluginterfaces/base/ibstream.h"
#include <cstdint>
#include <cstring>
namespace psychiatry::vst3 {
// Preserve the existing 32-bit little-endian wire format. IBStreamer checks
// byte count alone; this adapter also requires the actual host result.
class StateStream {
    Steinberg::IBStream* stream_;
public:
    explicit StateStream(Steinberg::IBStream* s) noexcept : stream_(s) {}
    bool readWord(std::uint32_t& v) {
        unsigned char b[4]{};
        Steinberg::int32 n=0;
        if(stream_->read(b,4,&n)!=Steinberg::kResultOk || n!=4)return false;
        v=std::uint32_t(b[0])|(std::uint32_t(b[1])<<8)|(std::uint32_t(b[2])<<16)|(std::uint32_t(b[3])<<24);
        return true;
    }
    bool writeWord(std::uint32_t v) {
        unsigned char b[]{static_cast<unsigned char>(v),static_cast<unsigned char>(v>>8),
                          static_cast<unsigned char>(v>>16),static_cast<unsigned char>(v>>24)};
        Steinberg::int32 n=0;
        return stream_->write(b,4,&n)==Steinberg::kResultOk && n==4;
    }
    template<class T> bool read(T& v) {
        static_assert(sizeof(T)==4);
        std::uint32_t u=0;
        if(!readWord(u))return false;
        std::memcpy(&v,&u,4);return true;
    }
    template<class T> bool write(T v) {
        static_assert(sizeof(T)==4);
        std::uint32_t u=0;std::memcpy(&u,&v,4);return writeWord(u);
    }
    bool readInt32(Steinberg::int32& v){return read(v);}
    bool readFloat(float& v){return read(v);}
    bool writeInt32(Steinberg::int32 v){return write(v);}
    bool writeFloat(float v){return write(v);}
    // Only a demonstrably empty legacy stream may select default UI state.
    bool empty() {
        Steinberg::int64 pos=0,end=0,restored=0;
        if(stream_->tell(&pos)!=Steinberg::kResultOk)return false;
        const auto result=stream_->seek(0,Steinberg::IBStream::kIBSeekEnd,&end);
        const auto restore=stream_->seek(pos,Steinberg::IBStream::kIBSeekSet,&restored);
        return result==Steinberg::kResultOk && restore==Steinberg::kResultOk &&
            restored==pos && end==0 && pos==0;
    }
};
}
