# Atom(Yuanzi) Programming Language Complier
Implementation on C.

# Build
> Unsupport `Windows`!

## Install 'libgetarg'
Follow [libgetarg.mk](lib/libgetarg.mk) to install libgetarg
and configuration makefile.

## Install 'libsctrie'
Like `Install 'libgetarg'` but see [libsctrie.mk](lib/libsctrie.mk).

## Build 'amc'
> Try read [Makefile](Makefile) to learn more build options.

### Normal version
```sh
make
```

### Debug version
```sh
make debug
```

# Others
## Unsupport 32-bit!
The file output by the compiler dosen't support 32-bit devices.
