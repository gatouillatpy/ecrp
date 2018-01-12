
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>

#include "compression.h"

using std::runtime_error;
using std::ostringstream;

//----------------------------------------------------------------------

namespace ecrp {
    
    string compress(const string &str, int compressionlevel) {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        if (deflateInit(&zs, compressionlevel) != Z_OK) {
            throw runtime_error("deflateInit failed while compressing.");
        }

        zs.next_in = (Bytef *)str.data();
        zs.avail_in = str.size();

        int ret;
        char outbuffer[65536];
        string outstring;

        do {
            zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = deflate(&zs, Z_FINISH);

            if (outstring.size() < zs.total_out) {
                outstring.append(outbuffer, zs.total_out - outstring.size());
            }
        } while (ret == Z_OK);

        deflateEnd(&zs);

        if (ret != Z_STREAM_END) {
            ostringstream oss;
            oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
            throw runtime_error(oss.str());
        }

        return outstring;
    }

    string decompress(char *data, size_t length) {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        if (inflateInit(&zs) != Z_OK) {
            throw runtime_error("inflateInit failed while decompressing.");
        }

        zs.next_in = (Bytef *)data;
        zs.avail_in = length;

        int ret;
        char outbuffer[65536];
        string outstring;

        do {
            zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = inflate(&zs, 0);

            if (outstring.size() < zs.total_out) {
                outstring.append(outbuffer, zs.total_out - outstring.size());
            }
        } while (ret == Z_OK);

        inflateEnd(&zs);

        if (ret != Z_STREAM_END) {
            ostringstream oss;
            oss << "Exception during zlib decompression: (" << ret << ") " << zs.msg;
            throw runtime_error(oss.str());
        }

        return outstring;
    }
}