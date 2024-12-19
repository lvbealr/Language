#include <cstdio>

int compare(int a, int b) {
    if (a > b) {
        return a;
    }

    if (b >= a) {
        return b;
    }

    return 0;
}

int main() {
    int x = 242;
    int y = 228;

    int result = compare(x, y);

    printf("%d", result);

    return 0;
}


