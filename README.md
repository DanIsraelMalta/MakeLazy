trying to add lazy evaluated operator overloading syntax to a given container without modifing it?
no problem, thats the reason MakeLazy was created for, just wrap a given container with it and compile...

simple example usage:
```c

using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
								 std::chrono::high_resolution_clock,
								 std::chrono::steady_clock>;
								 
// standard vector holding a lot of strings
std::vector<std::string> a(1'000'000),
                         b(1'000'000),
                         c(1'000'000),
                         d(1'000'000);
std::fill(a.begin(), a.end(), "expression ");
std::fill(b.begin(), b.end(), "template ");
std::fill(c.begin(), c.end(), "rule!");
std::fill(d.begin(), d.end(), "993766dk");

// wrap vector to be lazy
Lazy::Container<decltype(a)> lazy_a(a),
                             lazy_b(b),
                             lazy_c(c),
                             lazy_d(d);

// lazy evaluation...
Clock::time_point m_start;
lazy_d += lazy_a + lazy_b + lazy_c;
const Clock::time_point end{ Clock::now() };
auto lazy_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();

// ... vs. element wise evaluation...
Clock::time_point m_start1;
for (std::size_t i{}; i < 1000000; ++i) {
    d[i] += a[i] + b[i] + c[i];
}
const Clock::time_point end1{ Clock::now() };
auto scalar_time = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - m_start1).count();

// check the difference...
auto diff = lazy_time - scalar_time;
bool temp = lazy_time < scalar_time;
std::cout << "lazy is " << std::string(temp ? "faster" : "slower");
```

remarks:
* the wrapped container must include the following:
   - 'size' method (which returns the amount of elements in collection).
   - '[]' operator which allow access to an element via an index.
* the underlying element held by the wrapped container must support the overloaded operations.
* MakeLazy is not so usefull when handling containers of numerical varaibles. The reason is that
   the compiler might optimize this loops to use SIMD intrinsics, while MakeLazy is not SIMD friendly.
