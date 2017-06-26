#ifndef _COMMON_H_1b75e39d_b78e_45b1_b88a_fd1647eda741_
#define _COMMON_H_1b75e39d_b78e_45b1_b88a_fd1647eda741_

#include "MyLibrary.h"

#define KEY_MASK TAG4('DNNZ')

inline BOOL MillerRabin(ULONG64 n, ULONG64 k)
{
    if(n == k) return TRUE;
    ULONG64 s, d, b, e, x;

    // Factor n-1 as d 2^s
    for(s = 0, d = n - 1; !(d & 1); s++)
            d >>= 1;

    // x = k^d mod n using exponentiation by squaring
    // The squaring overflows for n >= 2^32
    for(x = 1, b = k % n, e = d; e; e >>= 1)
    {
            if(e & 1) x = (x * b) % n;
            b = (b * b) % n;
    }

    // Verify k^(d 2^[0…s-1]) mod n != 1
    if(x == 1 || x == n-1) return TRUE;
    while(s-- > 1)
    {
            x = (x * x) % n;
            if(x == 1) return FALSE;
            if(x == n-1) return TRUE;
    }
    return FALSE;
}

inline BOOL IsPrime(ULONG n)
{
    return (n > 73 &&
            !(n % 2 && n % 3 && n % 5 && n % 7 &&
              n % 11 && n % 13 && n % 17 && n % 19 && n % 23 && n % 29 &&
              n % 31 && n % 37 && n % 41 && n % 43 && n % 47 && n % 53 &&
              n % 59 && n % 61 && n % 67 && n % 71 && n % 73)
            ) ? FALSE:
                MillerRabin(n, 2) && MillerRabin(n, 7) && MillerRabin(n, 61);
}

#endif // _COMMON_H_1b75e39d_b78e_45b1_b88a_fd1647eda741_
