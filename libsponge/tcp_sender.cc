// #include "tcp_sender.hh"

// #include "tcp_config.hh"

// #include <random>

// // Dummy implementation of a TCP sender

// // For Lab 3, please replace with a real implementation that passes the
// // automated checks run by `make check_lab3`.

// template <typename... Targs>
// void DUMMY_CODE(Targs &&... /* unused */) {}

// using namespace std;

// //! \param[in] capacity the capacity of the outgoing byte stream
// //! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
// //! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
// TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
//     : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
//     , _initial_retransmission_timeout{retx_timeout}
//     , _stream(capacity) 
//     , _rto(retx_timeout) {}

// void TCPSender::pop_outstanding_segment() {
//     while(!_outstanding_segment.empty()) {
//         TCPSegment seg = _outstanding_segment.front();
//         uint64_t abs_seq = unwrap(seg.header().seqno,_isn,_next_seqno);
//         if(abs_seq + seg.length_in_sequence_space() <= _ackno)  {
//             _bytes_in_flight -= seg.length_in_sequence_space();
//             _outstanding_segment.pop();
//         } else break;
//     }
// }

// uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

// void TCPSender::fill_window() {
//     size_t window_size = _window_size;
//     _window_size = _window_size ? _window_size :1;
//     if(_ackno + window_size <= _next_seqno) return;

//     window_size = _ackno + window_size - _next_seqno;

//     size_t MAX_PAYLOAD_SIZE = TCPConfig::MAX_PAYLOAD_SIZE;

//     /*
//         fill_window when CLOSED or SYN_SENT or SYN_ACKED or SYN_ACKED_FIN_TO_SEND
//     */

//     while(window_size > 0 && 
//         (CLOSED() 
//         || ((SYN_SENT() || SYN_ACKED()) && !stream_in().buffer_empty()) 
//         || SYN_ACKED_FIN_TO_SEND())){
            
//         size_t used = 0;
        
//         uint64_t abs_seq;
//         abs_seq = _syn + _stream.bytes_read();

//         TCPSegment seg;

//         // Set SYN when CLOSED
//         if((used + 1 <= window_size) && CLOSED()) {
//             _syn = true;
//             seg.header().syn = true;
//             used += 1;
//         }
        
//         seg.header().seqno = wrap(abs_seq, _isn);
        
//         if(window_size > used) {
//             seg.payload() = Buffer(_stream.read(min(window_size - used, MAX_PAYLOAD_SIZE)));
//             used += seg.payload().size();
//         }

//         // Set FIN when SYN_ACKED_FIN_TO_SEND
//         if((used + 1 <= window_size) && SYN_ACKED_FIN_TO_SEND()) {
//             seg.header().fin = true;
//             _fin = true;
//             used += 1;
//         }

//         window_size -= used;

//         _segments_out.push(seg);
//         _outstanding_segment.push(seg);
//         _bytes_in_flight += seg.length_in_sequence_space();
//         _next_seqno = abs_seq + seg.length_in_sequence_space();

//         if(_timer.is_stopped()) _timer.set_new_timer(_rto);
//     }
// }

// //! \param ackno The remote receiver's ackno (acknowledgment number)
// //! \param window_size The remote receiver's advertised window size
// bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
//     DUMMY_CODE(ackno, window_size); 
//     uint64_t new_ackno = unwrap(ackno, _isn, _ackno);
//     if(new_ackno > next_seqno_absolute()) return false;
//     _window_size = window_size;
//     if(new_ackno <= _ackno) return true;
//     _rto = _initial_retransmission_timeout;
//     _consecutive_retransmissions = 0;
//     _ackno = new_ackno;
//     pop_outstanding_segment();
//     fill_window();
//     if(!_outstanding_segment.empty()) _timer.set_new_timer(_rto);
//     else _timer.stop();
//     return true;
// }

// //! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
// void TCPSender::tick(const size_t ms_since_last_tick) { 
//     DUMMY_CODE(ms_since_last_tick);
//     _timer.time_expired(ms_since_last_tick);
//     if(_timer.is_expired() && !_outstanding_segment.empty()) {
//         if(_window_size > 0) {
//             _consecutive_retransmissions++;
//             _rto <<= 1;
//         }
//         if(_consecutive_retransmissions <= TCPConfig::MAX_RETX_ATTEMPTS) 
//             _segments_out.push(_outstanding_segment.front());
//         _timer.set_new_timer(_rto);
//     }
// }

// unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

// void TCPSender::send_empty_segment() {
//     TCPSegment tmp;
//     // using next_seqno as seqno of empty_segment
//     tmp.header().seqno = wrap(next_seqno_absolute(), _isn);
//     _segments_out.push(tmp);
// }

#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

#include <iostream>
// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) 
    , _rto(retx_timeout) {}

void TCPSender::pop_outstanding(){
    while(!_outstanding_segment.empty()){
        TCPSegment seg = _outstanding_segment.front();
        uint64_t abs_seq = unwrap(seg.header().seqno,_isn,_next_seqno);
        if(abs_seq+seg.length_in_sequence_space()<=_ackno){
            _bytes_in_flight -= seg.length_in_sequence_space();
            _outstanding_segment.pop();
        }
        else {
            break;
        }
    }
}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {

    size_t windows_size = _win_size;

    windows_size = windows_size!=0 ? windows_size : 1;
    if(_ackno + windows_size <= _next_seqno) return ; 
    windows_size = _ackno + windows_size - _next_seqno;

   while(windows_size > 0 &&
        (CLOSED()
        ||((SYN_SENT()||SYN_ACKED())&&!stream_in().buffer_empty())
        || SYN_ACKED_FIN_TO_SEND())){

        size_t used = 0;

        uint64_t abs_seq_to_send = _syn + _stream.bytes_read();

        TCPSegment seg ;

        if((used + 1 <= windows_size)&&CLOSED()){// syn not send
            _syn = true;
            seg.header().syn = true;
            used+=1;
        }

        seg.header().seqno = wrap(abs_seq_to_send,_isn);

        if(windows_size > used){
            seg.payload() = Buffer(_stream.read(min(windows_size - used, TCPConfig::MAX_PAYLOAD_SIZE)));//windows_size 
            used+= seg.payload().size();
        }

        if((used+1<=windows_size) && SYN_ACKED_FIN_TO_SEND()){
            seg.header().fin = true;
            _fin = true;
            used +=1 ;
        }

        windows_size -= used;

        seg.header().seqno = wrap(abs_seq_to_send, _isn);
        _segments_out.push(seg);
        _outstanding_segment.push(seg);
        _bytes_in_flight += seg.length_in_sequence_space();
        _next_seqno =abs_seq_to_send+ seg.length_in_sequence_space();

        if(_timer.is_stop()){
            _timer.set_new_timer(_rto);
        }
    }

}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    uint64_t abs_seq = unwrap(ackno,_isn,_next_seqno);
    if(abs_seq>_next_seqno) return false;
    _win_size = window_size;
    if(abs_seq <= _ackno) return true;

    _rto = _initial_retransmission_timeout;
    _consecutive_retransmissions = 0;

    _ackno = abs_seq;
    pop_outstanding();
    fill_window();

    if(!_outstanding_segment.empty()) _timer.set_new_timer(_rto);
    else _timer.stop();

    return true;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    _timer.time_expired(ms_since_last_tick);

    if(_timer.is_expired()&&!_outstanding_segment.empty()){
            _rto *=2;
            // std::cout<< " after rto *2 "<<endl;
            _consecutive_retransmissions++;

        if(_consecutive_retransmissions<=TCPConfig::MAX_RETX_ATTEMPTS){
            _segments_out.push(_outstanding_segment.front());
        }
        _timer.set_new_timer(_rto);
    }

}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPSegment tmp;
    // using next_seqno as seqno of empty_segment
    tmp.header().seqno = wrap(next_seqno_absolute(), _isn);
    _segments_out.push(tmp);
}