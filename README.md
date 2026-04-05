This is my implementation of a HTTP 1.0 server. I used [this](www.w3.org/Protocols/HTTP/1.0/draft-ietf-http-spec.html) website for details on the standard.
[This guide](https://beej.us/guide/bgnet/html/split/) was very helpful for the socket programming portion of this project.

The server works and is mostly compliant to the 1.0 spec. I wasn't extremely rigorous about following it 100% (for example, it cannot understand HTTP 0.9 style requests),
but for the most part it complies to the standard.

The server is compatible with UNIX-like systems.

Also includes a truly next-generation website served by the HTTP server.

## Running

Build with cmake and try going to `localhost:8080` on your browser!

## Testing

I've included a rudimentary test suite to make sure that some harder to catch HTTP1.X functionality works as expected.

To run tests, use `ctest --test-dir build -V`
