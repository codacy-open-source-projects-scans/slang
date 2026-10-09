\function{pcre2_compile}
\synopsis{Compile a PCRE regular expression}
\usage{PCRE2_Type pcre2_compile (String_Type pattern [, Int_Type options])}
\description
 The \ifun{pcre2_compile} function compiles a PCRE style regular expression
 and returns the result.  The optional \var{options} argument may be used
 to provide addition information affecting the compilation of the pattern.
 Specifically, it is a bit-mapped value formed from the logical-or of zero
 or more of the following symbolic constants:
#v+
    PCRE2_ANCHORED     Force the match to be at the start of a string
    PCRE2_CASELESS     Matches are to be case-insensitive
    PCRE2_DOLLAR_ENDONLY (See PCRE docs for more information)
    PCRE2_DOTALL       The dot pattern matches all characters
    PCRE2_EXTENDED     Ignore whitespace in the pattern
    PCRE2_EXTRA        (See PCRE docs for features this activates)
    PCRE2_MULTILINE    Treat the subject string as multi-lines
    PCRE2_UNGREEDY     Make the matches not greedy
    PCRE2_UTF          Regard the pattern and subject strings as UTF-8
#v-
 Many of these flags may be set within the pattern itself.   See the PCRE
 library documentation for more information about the precise details
 of these flags and the supported regular expressions.

 Upon success, this function returns a \dtype{PCRE2_Type} object representing
 the compiled patterned.  If compilation fails, a \exc{ParseError}
 exception will be thrown.
\seealso{pcre2_exec, pcre2_nth_match, pcre2_nth_substr, pcre2_matches}
\done

\function{pcre2_exec}
\synopsis{Match a string against a compiled PCRE pattern}
\usage{Int_Type pcre2_exec(p, str [,pos [,options]])};
#v+
   PCRE2_Type p;
   String_Type str;
   Int_Type pos, options;
#v-
\description
 The \ifun{pcre2_exec} function applies a pre-compiled pattern \var{p} to a
 string \ifun{str} and returns the result of the match.  The optional third
 argument \ifun{pos} may be used to specify the point, as an offset from the
 start of the string, where matching is to start.  The fourth argument, if
 present, may be used to provide additional information about how matching
 is to take place.  Its value may be specified as a logical-or of zero or
 more of the following flags:
#v+
   PCRE2_NOTBOL
        The first character in the string is not at the beginning of a line.
   PCRE2_NOTEOL
        The last character in the string is not at the end of a line.
   PCRE2_NOTEMPTY
        An empty string is not a valid match.
#v-
 See the PCRE library documentation for more information about the meaning
 of these flags.

 Upon success, this function returns a positive integer equal to 1 plus the
 number of so-called captured substrings.  It returns 0 if the pattern
 fails to match the string.
\seealso{pcre2_compile, pcre2_nth_match, pcre2_nth_substr, pcre2_matches}
\done

\function{pcre2_matches}
\synopsis{Match a PCRE to a string and return the matches}
\usage{String_Type[] = pcre2_matches (regexp, str [,pcre2_exec_options])}
\description
  This function combines the \ifun{pcre2_exec} and
 \ifun{pcre2_nth_substr} functions into simple to use function that
 matches the specified regular expression \exmp{regexp} to the string
 \exmp{string} and returns matched substrings as an array.  If
 \exmp{regexp} is a string, the function will first compile the
 pattern using the \ifun{pcre2_compile} function.  The third parameter
 is optional and, if provided, will be passed as the \exmp{options}
 parameter to \ifun{pcre2_exec}.

 If no match is found, the function will return NULL.
\qualifiers
 The following qualifiers are supported:
#v+
   options=int        Options to pass to pcre2_compile
   offset=int         Start matching offset in bytes for pcre2_exec
#v-
\example
 After the execution of:
#v+
    str = "Error in file foo.c, line 127, column 10";
    pattern = "file ([^,]+), line (\\d+)";
    matches = pcre2_matches (pattern, str);
#v-
 the value of the matches variable will be the array:
#v+
    [ "file foo.c, line 127",
      "foo.c",
      127
    ]
#v-
\seealso{pcre2_compile, pcre2_exec, pcre2_nth_substr, pcre2_nth_match}
\done

\function{pcre2_nth_match}
\synopsis{Return the location of the nth match of a PCRE}
\usage{Int_Type[2] pcre2_nth_match (PCRE2_Type p, Int_Type nth)}
\description
 The \ifun{pcre2_nth_match} function returns an integer array whose values
 specify the locations of the beginning and end of the \var{nth} captured
 substrings of the most recent call to \ifun{pcre2_exec} with the compiled
 pattern.  A value of \var{nth} equal to 0 represents the substring
 representing the entire match of the pattern.

 If the \var{nth} match did not take place, the function returns \NULL.
\example
 After the execution of:
#v+
    str = "Error in file foo.c, line 127, column 10";
    pattern = "file ([^,]+), line (\\d+)";
    p = pcre2_compile (pattern);
    if (pcre2_exec (p, str))
      {
         match_pos = pcre2_nth_match (p, 0);
         file_pos = pcre2_nth_match (p, 1);
         line_pos = pcre2_nth_match (p, 2);
      }
#v-
 \exmp{match_pos} will be set to \exmp{[9,29]}, \exmp{file_pos} to \exmp{[14,19,]}
 and \exmp{line_pos} to \exmp{[26,29]}.  These integer arrays may be used to
 extract the substrings matched by the pattern, e.g.,
#v+
     file = substr (str, file_pos[0]+1, file_pos[1]-file_pos[0]);
     line = str[[line_pos[0]:line_pos[1]-1]];
#v-
 Alternatively, the function \ifun{pcre2_nth_substr} may be used to get the
 matched substrings:
#v+
     file = pcre2_nth_substr (p, str, 0);
#v-
\seealso{pcre2_compile, pcre2_exec, pcre2_nth_substr, pcre2_matches}
\done

\function{pcre2_nth_substr}
\synopsis{Extract the nth substring from a PCRE match}
\usage{String_Type pcre2_nth_substr (PCRE2_Type p, String_Type str, Int_Type nth)}
\description
 This function may be used to extract the \var{nth} captured substring
 resulting from the most recent use of the compiled pattern \var{p} by the
 \ifun{pcre2_exec} function.  Unlike \ifun{pcre2_nth_match}, this function returns
 the specified captured substring itself and not the position of the substring.
 For this reason, the subject string of the pattern is a required argument.
\seealso{pcre2_matches, pcre2_compile, pcre2_exec, pcre2_nth_match}
\done

\function{slang_to_pcre2}
\synopsis{Convert a S-Lang regular expression to a PCRE2 one}
\usage{String_Type slang_to_pcre (String_Type pattern)}
\description
 This function may be used to convert a slang regular expression to a
 PCRE2 compatible one.  The converted pattern is returned.
\seealso{pcre2_compile, string_match}
\done
