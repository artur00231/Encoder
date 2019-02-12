# Encoder
This is a simple library to change the text encoding.
The library itself generates, in compile time, required encoders by combining existing base encoders.
Currently the library contains 4 encodings (ASCII, UTF8, UTF16, URLEncode) and 6 base encoders.
It is very easy to extend, you just need to add new encoding, base endoders, and library will generate every thing else.
