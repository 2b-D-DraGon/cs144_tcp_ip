#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {

    bool old_fin_received = fin_state;

    if(seg.header().syn){
        if(syn_state){
            return false;
        }
        syn_state = true;
        isn = seg.header().seqno;
    }

    if(!seg.header().syn&&!syn_state){
        return false;
    }

    uint64_t win_start = unwrap(*ackno(),isn,_checkpoint);
    uint64_t win_size = window_size()?window_size():1;
    uint64_t win_end = win_start + win_size - 1;

    uint64_t seq_start = unwrap(seg.header().seqno,isn,_checkpoint);
    uint64_t seq_size = seg.length_in_sequence_space();
    seq_size = (seq_size) ? seq_size : 1;

    uint64_t payload_size = seg.payload().str().size();
    payload_size = (payload_size) ? payload_size : 1;
    uint64_t payload_end = seq_start + payload_size - 1;

    bool ifinbound=(seq_start>=win_start&&seq_start<=win_end)||(payload_end>=win_start&&payload_end<=win_end);
    if(ifinbound){
        _reassembler.push_substring(seg.payload().copy(),seq_start-1,seg.header().fin);
        _checkpoint=_reassembler.next_wanted_index();
    }
    
    if(seg.header().fin&&!fin_state){
        fin_state = true;
        if (seg.header().syn && seg.length_in_sequence_space() == 2) {
            stream_out().end_input();
        }
    }

    bool isfinished = fin_state&&(_reassembler.unassembled_bytes()==0);
    _ackno = wrap(_reassembler.next_wanted_index()+1+isfinished,isn);


    if (ifinbound ||(seg.header().syn)||(seg.header().fin&& !old_fin_received)) {
        return true;
    }
    return false;

}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(!syn_state) return nullopt;
    else{
        return _ackno;
    }
}

size_t TCPReceiver::window_size() const { return this->stream_out().remaining_capacity(); }
