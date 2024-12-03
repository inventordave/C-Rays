# C port of my once-active and operating Javascript-implemented Raytracer, RTDT.
# The Javascript RTDT took some serious hits from drunken nights of unmanaged refactoring,
# so I decided to revisit it, and in the process convert it to C for direct interoperability with
# DAVELIB and Barnyard.

# The Story So Far.
# -----------------

# The scalar vectors can be arbitrary-precision, thanks to integration with the core virtual operators
# and operations from APLIB, my concurrently-developing arbitrary-precision library.

