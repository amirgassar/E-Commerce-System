#include <iostream>
using namespace std;
const long long MOD = 1000000007;
long long mod_pow(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp /= 2;
    }
    return result;
}

int main() {
    int t;
    cin >> t;
    while (t--) {
        long long n;
        cin >> n;
        long long res = (mod_pow(2, n, MOD) + (n - 2)) % MOD;
        cout << res << '\n';
    }
    return 0;
}
