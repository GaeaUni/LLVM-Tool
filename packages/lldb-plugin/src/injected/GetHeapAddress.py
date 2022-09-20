import ctypes

# def malloc_zone_from_ptr(ptr):
#     libc = ctypes.CDLL(ctypes.util.find_library("c"))
#     f = libc.malloc_zone_from_ptr
#     f.argtypes = [ctypes.c_void_p]
#     f.restype = ctypes.c_void_p
#     return f(ptr)
