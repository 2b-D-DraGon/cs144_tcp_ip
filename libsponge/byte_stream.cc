#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <stdexcept>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) 
    :buffer(),_buffer_capacity(capacity), _total_bytes_read(0),_total_bytes_write(0),_end_input(false),_error(false){}

size_t ByteStream::write(const string &data) {
    
    if(_end_input) {return 0;}
    
    size_t _bytes_written = min(data.size(),_buffer_capacity-buffer.size());
    _total_bytes_write+= _bytes_written;
    for(size_t i=0;i<_bytes_written;++i){
        buffer.push_back(data[i]);
    }
    return _bytes_written;

}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    
    size_t pop_size = min(buffer.size(),len);
    
    string pop_string = string(buffer.begin(),buffer.begin()+pop_size);

    return pop_string;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) 
{ 
    size_t pop_size = min(buffer.size(),len);
    _total_bytes_read += pop_size;

    for(size_t i=0 ; i<pop_size ;++i){
        buffer.pop_front();
    }
}

string ByteStream::read(const size_t len) {
        string ret = peek_output(len);
        pop_output(len);
        return ret;
}

void ByteStream::end_input() { _end_input = true;}

bool ByteStream::input_ended() const { return _end_input; }

size_t ByteStream::buffer_size() const { return buffer.size(); }

bool ByteStream::buffer_empty() const { return buffer.empty(); }

bool ByteStream::eof() const { return _end_input && buffer.empty(); }

size_t ByteStream::bytes_written() const { return _total_bytes_write; }

size_t ByteStream::bytes_read() const { return _total_bytes_read; }

size_t ByteStream::remaining_capacity() const { return _buffer_capacity-buffer.size(); }
