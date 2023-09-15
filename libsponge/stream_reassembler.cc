#include "stream_reassembler.hh"

#include <cassert>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _unassemble_buf()
    , _next_assembled_idx(0)
    , _bytes_unassembled_num(0)
    , _eof(-1)
    , _output(capacity)
    , _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {

    auto index_ptr = _unassemble_buf.upper_bound(index);
    if(index_ptr!=_unassemble_buf.begin()){
        index_ptr--;
    }

    size_t new_index = index;
    if (index_ptr != _unassemble_buf.end() && index_ptr->first <= index) {
        const size_t prev_index = index_ptr->first;

        if(index<prev_index+index_ptr->second.size()){
            new_index = prev_index+index_ptr->second.size();
        }
    }
    else if (index < _next_assembled_idx)
        new_index = _next_assembled_idx;

    const size_t data_start_pos = new_index - index;
    ssize_t data_size = data.size() - data_start_pos;

    index_ptr = _unassemble_buf.lower_bound(new_index);

    while(index_ptr!=_unassemble_buf.end()&&new_index<=index_ptr->first){
        const size_t data_end_pos = new_index+data_size;

        if(index_ptr->first<data_end_pos){
            if(index_ptr->first+index_ptr->second.size()>data_end_pos){
                data_size = index_ptr->first-new_index;
                break;
            }
            else{
                _bytes_unassembled_num -= index_ptr->second.size();
                index_ptr=_unassemble_buf.erase(index_ptr);
                continue;
            }
        }
        else{
            break;
        }

    }
    
    size_t unacceptable_index = _next_assembled_idx+_capacity-_output.buffer_size();
    if (unacceptable_index <= new_index)
        return;

    if(data_size>0){
        const string new_data = data.substr(data_start_pos,data_size);

        if(new_index==_next_assembled_idx){
            const size_t bytes_written= _output.write(new_data);
            _next_assembled_idx += bytes_written;
            if(bytes_written<new_data.size()){
                // const string to_unassembled_str = new_data.substr(bytes_written,new_data.size()-bytes_written);
                const string to_unassembled_str = new_data.substr(bytes_written);                
                _bytes_unassembled_num += to_unassembled_str.size();
                _unassemble_buf.insert(make_pair(_next_assembled_idx,to_unassembled_str));
            }
        }
        else{
            const string to_unassembled_str = new_data;
            _bytes_unassembled_num+=to_unassembled_str.size();
            _unassemble_buf.insert(make_pair(new_index,to_unassembled_str));
        }
    }

    for(auto ptr = _unassemble_buf.begin();ptr!=_unassemble_buf.end();){
        if(ptr->first==_next_assembled_idx){
            const size_t written_bytes = _output.write(ptr->second);
            _next_assembled_idx += written_bytes;

            if(written_bytes< ptr->second.size()){
                _bytes_unassembled_num -= written_bytes;
                _unassemble_buf.insert(make_pair(_next_assembled_idx,ptr->second.substr(written_bytes)));

                _unassemble_buf.erase(ptr);
                break;
            }
            _bytes_unassembled_num -= ptr->second.size();
            ptr = _unassemble_buf.erase(ptr);
        }
        else break;

    }

    if (eof)
        _eof = index + data.size();
    if (_eof <= _next_assembled_idx)
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const { return _bytes_unassembled_num; }

bool StreamReassembler::empty() const { return _bytes_unassembled_num == 0; }
