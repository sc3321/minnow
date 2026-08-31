#include <exception>
#include <locale>
#include <queue>
#include <stdexcept>
#include <string_view>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
    if(available_capacity() == 0) {return;}
    if(data == "") {return;}
    if(data.size() > available_capacity()){
        data.resize(available_capacity());
        bytes.push(data);
        bytes_pushed_total += data.size();
    }
    else{
        bytes.push(data);
        bytes_pushed_total += data.size();
    }

}

void Writer::close()
{
    is_stream_closed = true;
}

void Writer::set_error()
{
   error = true;
}

bool Writer::is_closed() const
{
    return is_stream_closed;
}

uint64_t Writer::available_capacity() const
{
    return capacity_ - (bytes_pushed_total - bytes_popped_total);
}

uint64_t Writer::bytes_pushed() const
{
    return bytes_pushed_total;
}

string_view Reader::peek() const
{
    if(bytes.empty()){
        return {};
    }
    std::string_view peekingBytes = bytes.front();
    if(peekingBytes.size() >= offset){
        peekingBytes.remove_prefix(offset);
    }
    peekingBuffer.clear();
    std::queue<std::string> bytesCopy = bytes;
    bytesCopy.pop();
    peekingBuffer += peekingBytes;
    while(peekingBuffer.size() <= (bytes_buffered() - peekingBytes.size())){
       peekingBuffer += bytesCopy.front();
        bytesCopy.pop();
    }
    return peekingBuffer;
}

bool Reader::is_finished() const
{
    return is_stream_closed && (bytes_pushed_total == bytes_popped_total);
}

bool Reader::has_error() const
{
    return error;
}

void Reader::pop( uint64_t len )
{  
    if(len == 0) {return;}
    if(len > bytes_buffered()) {
        len = (bytes_buffered());
    }
    while(len > 0 && !bytes.empty()){
        if(len >= bytes.front().size() - offset){
            std::string_view front = bytes.front();
            bytes_popped_total += (front.size() - offset);
            len -= (front.size() - offset);
            offset = 0;
            bytes.pop();
        }
        else{
            offset += len;
            bytes_popped_total += len;
            len = 0;
        }
    }
}

uint64_t Reader::bytes_buffered() const
{
    return bytes_pushed_total - bytes_popped_total;
}

uint64_t Reader::bytes_popped() const
{
    return bytes_popped_total;
}
