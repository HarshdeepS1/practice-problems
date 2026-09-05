void* my_memmove(void* dest, const void* src, size_t n) {
    if (dest == src) return dest;

    uint8_t* d = (uint8_t*) dest;
    const uint8_t* s = (const uint8_t*) src;
    // If dest falls between [src,src+n]    
    if (d > s && d < (s + n)) {
        // Copy backwards
        for (size_t i=n; i>0; i--) {
            d[i-1] = s[i-1]; 
        }
        return dest;
    }

    // Everything else use forwards copying

    for (size_t i=0; i<n; i++) {
        d[i] = s[i];
    }

    return dest;
}