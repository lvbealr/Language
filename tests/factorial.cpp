#include <cstdio>

int factorial(int x) {
    if (x == 1) {
        return 1;
    }

    return x * factorial(x-1);
}

int main() {
    int x = 0;
    scanf("%d", &x);

    int y = factorial(x);
    printf("%d", y);
}
