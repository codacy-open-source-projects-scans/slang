\function{zlib_deflate}
\synopsis{Compress a string}
\usage{zstr = zlib_deflate (str)}
\description
\qualifiers
\qualifier{level=val}{}
\qualifier{method=val}{}
\qualifier{wbits=val}{}
\qualifier{memlevel=val}{}
\qualifier{strategy=val}{}
\example
\notes
\seealso{zlib_inflate, zlib_inflate_new}
\done


\function{zlib_deflate_new}
\synopsis{Create a zlib deflation object}
\usage{zlibobj = zlib_deflate_new ()}
\description
\qualifiers
\qualifier{level=val}{one of: ZLIB_NO_COMPRESSION, ZLIB_BEST_SPEED,
ZLIB_BEST_COMPRESSION,
ZLIB_DEFAULT_COMPRESSION}{ZLIB_DEFAULT_COMPRESSION}
\qualifier{method=val}{ZLIB_DEFLATED}{ZLIB_DEFLATED}
\qualifier{wbits=val}{Window size in bits, 8-15 is recommended}{15}
\qualifier{memlevel=val}{Specifies the amount of memory to use; 1-9,
default=8}
\qualifier{strategy=val}{one of: ZLIB_DEFAULT_STRATEGY, ZLIB_FILTERED,
ZLIB_HUFFMAN_ONLY, ZLIB_RLE,ZLIB_FIXED}{ZLIB_DEFAULT_STRATEGY}
\methods
\method{.deflate}{compress a string}
\method{.reset}{Resets the stream keeping various state parameters}
\method{.flush}{Flush pending output to the output buffer}
\example
\notes
 See the zlib compression library documentation for more information
 regarding the qualifiers and methods.  Note that this module uses ZLIB as a
 prefix whereas the underlying library API uses Z, e.g.,
 \exmp{ZLIB_FIXED} vs \exmp{Z_FIXED}.
\seealso{zlib_deflate, zlib_inflate_new}
\done

\function{zlib_inflate}
\synopsis{Uncompress a string}
\usage{str = zlib_inflate (zstr)}
\description
\qualifiers
\qualifier{wbits=val}{}
\example
\notes
\seealso{}
\done


\function{zlib_inflate_new}
\synopsis{Create a zlib inflation object}
\usage{}
\description
\methods
\method{inflate}{}
\method{reset}{}
\method{flush}{}
\example
\notes
\seealso{}
\done
