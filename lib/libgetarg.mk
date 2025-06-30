# Clone 'libgetarg' into lib directory or install it to system.
# `git clone https://github.com/at2er/libgetarg`
# Don't forget compile it! Use `make` to get 'libgetarg.a'
# If 'libgetarg' is in lib directory
# then disable comment this line and comment the next line.
#LIBGETARG = -lgetarg -Llib/libgetarg -Ilib/libgetarg
LIBGETARG = -lgetarg
