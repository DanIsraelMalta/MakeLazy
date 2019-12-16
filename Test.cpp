#include "MakeLazy.h"

#include<vector>
#include<array>
#include<tuple>
#include<chrono>
#include<iostream>

using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock>;

struct Element {
    std::int32_t m_int{};
    float m_float{};
    std::string m_string;

    Element() = default;
    Element(const std::int32_t i, const float f, const std::string& s) : m_int(i), m_float(f), m_string(s) {}
    Element(const Element&) = default;
    Element& operator=(const Element&) = default;
    Element(Element&&) noexcept = default;
    Element& operator =(Element&&) noexcept = default;

    Element& operator += (const Element& other) {
        m_int += other.m_int;
        m_float += other.m_float;
        m_string += other.m_string;
        return *this;
    }

    friend Element operator + (Element lhs, const Element& other) {
        lhs += other;
        return lhs;
    }
};

int main() {
    
    // test a simple case with std::string
    {
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

        // test lazy evaluation
        Clock::time_point m_start;
        lazy_d += lazy_a + lazy_b + lazy_c;
        const Clock::time_point end{ Clock::now() };
        auto lazy_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();

        // element wise evaluation
        Clock::time_point m_start1;
        for (std::size_t i{}; i < 1000000; ++i) {
            d[i] += a[i] + b[i] + c[i];
        }
        const Clock::time_point end1{ Clock::now() };
        auto scalar_time = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - m_start1).count();
        auto diff = lazy_time - scalar_time;
        bool temp = lazy_time < scalar_time;
        std::cout << "lazy is " << std::string(temp ? "faster" : "slower");
    }
    
    // test a case with container holding a complex structure
    {
        // stack based containers holding 'Elements'
        std::array<Element, 100> avt,
                                 bvt,
                                 cvt,
                                 dvt;
        std::fill(avt.begin(), avt.end(), Element{325, -15.0f, "hi"});
        std::fill(bvt.begin(), bvt.end(), Element{-325, +15.0f, " expression "});
        std::fill(cvt.begin(), cvt.end(), Element{0, 1.0f, "template"});
        std::fill(dvt.begin(), dvt.end(), Element{0, 0.0f, "__"});

        // lazy wrappers
        Lazy::Container<decltype(avt)> lazy_a(avt),
                                       lazy_b(bvt),
                                       lazy_c(cvt),
                                       lazy_d(dvt);

        // test lazy evaluation
        Clock::time_point m_start;
        lazy_d += lazy_a + lazy_b + lazy_c;
        const Clock::time_point end{ Clock::now() };
        auto lazy_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();


        Clock::time_point m_start1;
        for (std::size_t i{}; i < 100; ++i) {
            dvt[i] += avt[i] + bvt[i] + cvt[i];
        }
        const Clock::time_point end1{ Clock::now() };
        auto scalar_time = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - m_start1).count();
        auto diff = lazy_time - scalar_time;
        bool temp = lazy_time < scalar_time;
        std::cout << "lazy is " << std::string(temp ? "faster" : "slower");
    }
    
    return 1;
}
