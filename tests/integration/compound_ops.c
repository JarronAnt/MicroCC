int main() {
    int x = 27;
    x %= 7;
    x <<= 2;
    x |= 3;
    x &= 15;
    x ^= 5;
    x >>= 1;
    x /= 2;
    x *= 3;
    return x;
}
