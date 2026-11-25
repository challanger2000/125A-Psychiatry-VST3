#pragma once
#include <array>
#include <atomic>
#include <cstdint>
namespace psychiatry::vst3 {
struct StateFrame {
    std::array<float,14> values{};
    std::uint64_t generation=0;
};
// Single publisher, single reader (external control readers serialized).
// The publisher can always choose one of three slots: neither the published
// slot nor the pinned reader slot. Sequential consistency orders pinning,
// rechecking, and publication, including reuse of a previously published slot.
// The realtime reader attempts once; the control reader can retry off audio.
class StateMailbox {
    std::array<StateFrame,3> slots_{};
    mutable std::atomic<unsigned> pin_{3};
    std::atomic<unsigned> published_{0};
    static_assert(std::atomic<unsigned>::is_always_lock_free);
public:
    void publish(const StateFrame& frame) noexcept {
        const auto current=published_.load();
        const auto pinned=pin_.load();
        for(unsigned i=0;i<3;++i)if(i!=current&&i!=pinned){
            slots_[i]=frame;published_.store(i);return;
        }
    }
    bool tryRead(StateFrame& frame) const noexcept {
        const auto i=published_.load();
        pin_.store(i);
        if(published_.load()!=i){pin_.store(3);return false;}
        frame=slots_[i];pin_.store(3);return true;
    }
};
}
