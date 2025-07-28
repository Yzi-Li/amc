# Clone 'libsctrie' into lib directory or install it to system.
# `git clone https://github.com/at2er/libsctrie`
# Don't forget compile it! Use `make` to get 'libsctrie.a'
# If 'libsctrie' is in lib directory
# then disable comment this line and comment the next line.
#LIBSCTRIE = -lsctrie -Llib/libsctrie -Ilib/libsctrie
LIBSCTRIE = -lsctrie
