# Atom(Yuanzi) Programming Language Complier
Implementation on C.

# Install | Build
## Install 'libgetarg'
Follow [libgetarg.mk](lib/libgetarg.mk) to install libgetarg
and configuration makefile.

## Build 'amc'
### Normal version
```sh
make
```
### Debug version
```sh
make debug
```

## Misc
Try read [Makefile](Makefile) to learn more build options.

# BUGS
## Unsupport 32-bit!
The file output by the compiler dosen't support 32-bit devices.

# TODO
- [ ] 32-bit support (but, maybe it will be implement in YZC)
- [ ] syntax:
    - [ ] decorator.
- [ ] type:
    - [ ] pointer.
    - [ ] function pointer.
    - [ ] array.
    - [ ] string.
