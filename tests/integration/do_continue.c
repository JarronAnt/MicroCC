int main() {
    int i = 0, j = 0;
    do {
        i++;
        if (i < 3)
            continue;
        j += i;
    } while (i < 4);
    return j;
}
