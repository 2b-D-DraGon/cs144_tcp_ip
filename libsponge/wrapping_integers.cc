#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return WrappingInt32{isn+static_cast<uint32_t>(n)};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.

uint64_t abs_gap(uint64_t a, uint64_t b){
    return a<b?b-a:a-b;
}

uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t num;
    uint64_t abs_isn_checkpoint = 0xffffffff00000000 & checkpoint;

    if(n.raw_value()>=isn.raw_value()){
        num = n.raw_value()-isn.raw_value();
    }
    else{
        num = 0x0000000100000000-isn.raw_value()+n.raw_value();
    }

    uint64_t a = abs_isn_checkpoint+num;
    uint64_t b = a + 0x0000000100000000;
    uint64_t c = a - 0x0000000100000000;

    //b
    //a
    //c
    if(abs_gap(a,checkpoint)<abs_gap(b,checkpoint)){
        if(abs_gap(a,checkpoint)<abs_gap(c,checkpoint)){
            return a;
        }
        else return c;
    }
    else return b;
}
