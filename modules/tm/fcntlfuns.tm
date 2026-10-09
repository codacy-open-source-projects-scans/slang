\function{fcntl_getfd}
\synopsis{Get a file descriptor's flags}
\usage{flags = fcntl_getfd (fd)}
\description
 This function returns the flags associated with the open file
 descriptor \exmp{fd}, which must be either an \ifun{FD_Type} object
 or an integer.  If returns the value of the flags, or -1 and sets the
 value of \ivar{errno} if an error occurs.
\example
#v+
   flags = fcntl_getfd (fileno(stdout));
   if (flags == -1)
     () = fprintf (stderr, "An error occurred: %s\n", errno_string());
#v-
\notes
 This function is a wrapper around the OS \exmp{fcntl(fd, F_GETFD,...)}
 function.
\seealso{fcntl_setfd, fcntl_getfl, fcntl_getpipe_sz, fileno, _fileno}
\done


\function{fcntl_setfd}
\synopsis{Set a file descriptor's flags}
\usage{status = fcntl_setfd (flags, fd)}
\description
 This function may be used to set the flags on the open file
 descriptior \exmp{fd}.  Here \exmp{flags} is an integer and \exmp{fd}
 is either an integer or a \ifun{FD_Type} object.  It returns 0 if
 successful, or -1 and sets the value of \ivar{error} upon failure.
\example
#v+
  flags = fcntl_getfd (fd);
  status = fcntl_setfd (flags | FD_CLOEXEC, fd);
  if (status != 0)
     () = fprintf (stderr, "An error occurred: %s\n", errno_string());
#v-
\notes
 This function is a wrapper around the OS \exmp{fcntl(fd,F_SETFD,flags)}
 function.
\seealso{fcntl_getfd, fcntl_setfl, fcntl_setpipe_sz, fileno, _fileno}
\done


\function{fcntl_getfl}
\synopsis{}
\usage{}
\description
\example
\notes
\seealso{}
\done


\function{fcntl_setfl}
\synopsis{}
\usage{}
\description
\example
\notes
\seealso{}
\done


\function{fcntl_getpipe_sz}
\synopsis{}
\usage{}
\description
\example
\notes
\seealso{}
\done


\function{fcntl_setpipe_sz}
\synopsis{}
\usage{}
\description
\example
\notes
\seealso{}
\done
