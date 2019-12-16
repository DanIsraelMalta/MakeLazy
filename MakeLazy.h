/**
* container wrapper (by reference) with lazy-evaluated loop-fused element wise operator overload.
*
* Dan I. Malta
**/
#pragma once
#include <assert.h>
#include <type_traits>
#include <utility>
#include <functional>

namespace Lazy {

    /**
    * objects to decode lazy operations
    **/
    namespace detail {

        /**
        * \brief a binary expression
        *
        * @param {LeftExpr,  in} left side of expression
        * @param {BinaryOp,  in} binary operation
        * @param {RightExpr, in} right side of expression
        **/
        template<typename LeftExpr, typename BinaryOp, typename RightExpr>
        class BinaryExpression {

            // properties
            private:
                const LeftExpr  m_left;
                const RightExpr m_right;

            // constructors
            public:
                // prohibit empty constructor
                BinaryExpression() = delete;

                // element wise constructor
                BinaryExpression(LeftExpr l, RightExpr r) : m_left(std::forward<LeftExpr>(l)), m_right(std::forward<RightExpr>(r)) {}

                // expression can not be copied...
                BinaryExpression(const BinaryExpression&)             = delete;
                BinaryExpression& operator =(const BinaryExpression&) = delete;

                // ...only moved
                BinaryExpression(BinaryExpression&&) noexcept             = default;
                BinaryExpression& operator =(BinaryExpression&&) noexcept = default;

            // binary operators overload
            public:

#define CREATE_BINARY_EXPRESSION_OPERATOR(xi_operator)                                                                                                                                          \
        template<typename RE> auto operator xi_operator(RE&& re) const -> BinaryExpression<const BinaryExpression<LeftExpr, BinaryOp, RightExpr>&, BinaryOp, decltype(std::forward<RE>(re))> {  \
            return BinaryExpression<const BinaryExpression<LeftExpr, BinaryOp, RightExpr>&, BinaryOp, decltype(std::forward<RE>(re))>(*this, std::forward<RE>(re));                             \
        }

                CREATE_BINARY_EXPRESSION_OPERATOR(+= );
                CREATE_BINARY_EXPRESSION_OPERATOR(-= );
                CREATE_BINARY_EXPRESSION_OPERATOR(*= );
                CREATE_BINARY_EXPRESSION_OPERATOR(/= );
                CREATE_BINARY_EXPRESSION_OPERATOR(|= );
                CREATE_BINARY_EXPRESSION_OPERATOR(&= );
                CREATE_BINARY_EXPRESSION_OPERATOR(^= );
                CREATE_BINARY_EXPRESSION_OPERATOR(<<= );
                CREATE_BINARY_EXPRESSION_OPERATOR(>>= );
                CREATE_BINARY_EXPRESSION_OPERATOR(+);
                CREATE_BINARY_EXPRESSION_OPERATOR(-);
                CREATE_BINARY_EXPRESSION_OPERATOR(*);
                CREATE_BINARY_EXPRESSION_OPERATOR(/ );
                CREATE_BINARY_EXPRESSION_OPERATOR(| );
                CREATE_BINARY_EXPRESSION_OPERATOR(&);
                CREATE_BINARY_EXPRESSION_OPERATOR(^);
                CREATE_BINARY_EXPRESSION_OPERATOR(<< );
                CREATE_BINARY_EXPRESSION_OPERATOR(>> );
                CREATE_BINARY_EXPRESSION_OPERATOR(&&);
                CREATE_BINARY_EXPRESSION_OPERATOR(|| );
                CREATE_BINARY_EXPRESSION_OPERATOR(== );
                CREATE_BINARY_EXPRESSION_OPERATOR(!= );
                CREATE_BINARY_EXPRESSION_OPERATOR(< );
                CREATE_BINARY_EXPRESSION_OPERATOR(<= );
                CREATE_BINARY_EXPRESSION_OPERATOR(> );
                CREATE_BINARY_EXPRESSION_OPERATOR(>= );
#undef CREATE_BINARY_EXPRESSION_OPERATOR

            // getters
            public:

                // expression left hand side
                auto le()       -> typename std::add_lvalue_reference<                        LeftExpr>       ::type { return m_left; }
                auto le() const -> typename std::add_lvalue_reference<typename std::add_const<LeftExpr>::type>::type { return m_left; }

                // expression right hand side
                auto re()       -> typename std::add_lvalue_reference<                        RightExpr>       ::type { return m_right; }
                auto re() const -> typename std::add_lvalue_reference<typename std::add_const<RightExpr>::type>::type { return m_right; }

                // [] overload to get expression at a specific index
                auto operator [](std::size_t index) const -> decltype(BinaryOp::apply(this->le()[index], this->re()[index])) {
                    return BinaryOp::apply(le()[index], re()[index]);
                }
        };

        /**
        * binary operations (numerical/bit)
        **/
        namespace BinaryOperations {

#define CREATE_BINARY_OPERATION(xi_name, xi_operator, xi_assign_operator)                                      \
        template<typename T> struct xi_name {                                                                  \
            constexpr static T apply(const T& a, const T& b) { return a xi_operator b; }                       \
            constexpr static T apply(T&&      a, const T& b) { a xi_assign_operator b; return std::move(a); }  \
            constexpr static T apply(const T& a, T&&      b) { b xi_assign_operator a; return std::move(b); }  \
            constexpr static T apply(T&&      a, T&&      b) { a xi_assign_operator b; return std::move(a); }  \
        }

            CREATE_BINARY_OPERATION(ADD, +, +=);
            CREATE_BINARY_OPERATION(SUB, -, -=);
            CREATE_BINARY_OPERATION(MUL, *, *=);
            CREATE_BINARY_OPERATION(DIV, / , /=);
            CREATE_BINARY_OPERATION(LOR, | , |=);
            CREATE_BINARY_OPERATION(LAND, &, &=);
            CREATE_BINARY_OPERATION(LXOR, ^, ^=);
            CREATE_BINARY_OPERATION(SHL, << , <<=);
            CREATE_BINARY_OPERATION(SHR, >> , >>=);
#undef CREATE_BINARY_OPERATION

            // relation/logical operator overloading
#define CREATE_BINARY_OPERATION(xi_name, xi_operator)                                         \
        template<typename T> struct xi_name {                                                 \
            constexpr static bool apply(const T& a, const T& b) { return a xi_operator b; }   \
            constexpr static bool apply(T&&      a, const T& b) { return a xi_operator b; }   \
            constexpr static bool apply(const T& a, T&&      b) { return a xi_operator b; }   \
            constexpr static bool apply(T&&      a, T&&      b) { return a xi_operator b; }   \
        }

            CREATE_BINARY_OPERATION(AND, &&);
            CREATE_BINARY_OPERATION(OR, || );
            CREATE_BINARY_OPERATION(EQ, == );
            CREATE_BINARY_OPERATION(NEQ, != );
            CREATE_BINARY_OPERATION(LT, < );
            CREATE_BINARY_OPERATION(LE, <= );
            CREATE_BINARY_OPERATION(GT, > );
            CREATE_BINARY_OPERATION(GE, >= );
#undef CREATE_BINARY_OPERATION
        };
    };

    /**
    * concepts
    **/
    namespace Concepts {
        // test if an object is a binary expression
        template<typename>                           struct is_binary_expression                                    : std::false_type {};
        template<typename L, typename B, typename R> struct is_binary_expression<detail::BinaryExpression<L, B, R>> : std::true_type {};
        template<typename T> constexpr bool is_binary_expression_v = is_binary_expression<T>::value;

        // test if an object has the 'size()' method
        template<typename T, typename = void> struct has_size                                                : std::false_type {};
        template<typename T>                  struct has_size<T, decltype(std::declval<T>().size(), void())> : std::true_type  {};
        template<typename T> constexpr bool has_size_v = has_size<T>::value;

        // test if an object has '[]' operator
        template<typename T, typename = void> struct has_access_operator                                                 : std::false_type {};
        template<typename T>                  struct has_access_operator<T, std::void_t<decltype(std::declval<T>()[0])>> : std::true_type  {};
        template<typename T> constexpr bool hsa_access_operator_v = has_access_operator<T>::value;

        // test if an object can be wrapped by Lazy::Container
        template<typename T> constexpr bool can_be_wrapped = has_size_v<T> && hsa_access_operator_v<T>;
    }

    /**
    * \brief extend a container to be lazy evaluated under operator overloading.
    *
    * @param{COLLECTION} the collection/container to be (element wise) lazy evaluated.
    *                    the container must include the following:
    *                    > 'size' method (which returns the amount of elements in collection).
    *                    > '[]' operator which allow access to an element via an index.
    *
    * \remarks notice that the underlying element held by the wrapped container must support the
    *          overloaded operations.
    **/
    template<typename COLLECTION> struct Container {
        // test that container upholds basic requirement
        static_assert(Concepts::can_be_wrapped<COLLECTION>, "Container<Collection>: Collection can not be wrapped.");

        //
        // aliases
        //
        using value_type             = typename COLLECTION::value_type;
        using size_type              = typename COLLECTION::size_type;
        using difference_type        = typename COLLECTION::difference_type;
        using reference              = typename COLLECTION::reference;
        using const_reference        = typename COLLECTION::const_reference;
        using pointer                = typename COLLECTION::pointer;
        using const_pointer          = typename COLLECTION::const_pointer;
        using iterator               = typename COLLECTION::iterator;
        using const_iterator         = typename COLLECTION::const_iterator;
        using reverse_iterator       = typename COLLECTION::reverse_iterator;
        using const_reverse_iterator = typename COLLECTION::const_reverse_iterator;

        //
        // constructors
        // 

        // standard constructor
        constexpr Container(COLLECTION& xi_col) : m_container(xi_col) {}

        // construct from a binary expression
        template<typename T, typename std::enable_if<Concepts::is_binary_expression_v<T>>::type* = nullptr>
        Container(T&& xi_expression) noexcept {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] = std::move(xi_expression[i]);
            }
        }

        // assign from a (right) expression
        template<typename T, typename std::enable_if<Concepts::is_binary_expression_v<T>>::type * = nullptr>
        Container& operator =(T&& xi_expression) noexcept {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] = std::move(xi_expression[i]);
            }
            return *this;
        }

        // copy semantics
        Container(const Container&)              = default;
        Container& operator = (const Container&) = default;

        // move semantics
        Container(Container&&) noexcept              = default;
        Container& operator = (Container&&) noexcept = default;

        //
        // access operator
        //
        auto operator[] (typename size_type i)       { return m_container[i]; }
        auto operator[] (typename size_type i) const { return m_container[i]; }

        //
        // operator overloading
        //

        // '+'/'+=' overload 
        template<typename RightExpr, typename std::enable_if<std::is_convertible_v<RightExpr, value_type>>::type* = nullptr>
        Container& operator +=(RightExpr&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] += static_cast<value_type>(xi_expression);
            }
            return *this;
        }
        template<typename T, typename std::enable_if<Concepts::is_binary_expression_v<T>>::type * = nullptr>
        Container& operator +=(T&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] += std::move(xi_expression[i]);
            }
            return *this;
        }
        template<typename RightExpr> auto operator +(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::ADD<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::ADD<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '-'/'-=' overload 
        template<typename RightExpr, typename std::enable_if<std::is_convertible_v<RightExpr, value_type>>::type* = nullptr> 
        Container& operator -=(RightExpr&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] -= static_cast<value_type>(xi_expression);
            }
            return *this;
        }
        template<typename T, typename std::enable_if<Concepts::is_binary_expression_v<T>>::type* = nullptr>
        Container& operator -=(T&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] -= std::move(xi_expression[i]);
            }
            return *this;
        }
        template<typename RightExpr> auto operator -(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::SUB<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::SUB<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '*'/'*=' overload 
        template<typename RightExpr, typename std::enable_if<std::is_convertible_v<RightExpr, value_type>>::type* = nullptr>
        Container& operator *=(RightExpr&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] *= static_cast<value_type>(xi_expression);
            }
            return *this;
        }
        template<typename T, typename std::enable_if<Concepts::is_binary_expression_v<T>>::type* = nullptr>
        Container& operator *=(T&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] *= std::move(xi_expression[i]);
            }
            return *this;
        }
        template<typename RightExpr> auto operator *(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::MUL<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::MUL<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '/'/'/=' overload 
        template<typename RightExpr, typename std::enable_if<std::is_convertible_v<RightExpr, value_type>>::type* = nullptr>
        Container& operator /=(RightExpr&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] /= static_cast<value_type>(xi_expression);
            }
            return *this;
        }
        template<typename T, typename std::enable_if<Concepts::is_binary_expression_v<T>>::type* = nullptr>
        Container& operator /=(T&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] /= std::move(xi_expression[i]);
            }
            return *this;
        }
        template<typename RightExpr> auto operator /(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::DIV<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::DIV<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '&'/'&=' overload 
        template<typename RightExpr, typename std::enable_if<std::is_convertible_v<RightExpr, value_type>>::type* = nullptr>
        Container& operator &=(RightExpr&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] &= static_cast<value_type>(xi_expression);
            }
            return *this;
        }
        template<typename T, typename std::enable_if<Concepts::is_binary_expression_v<T>>::type* = nullptr>
        Container& operator &=(T&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] &= std::move(xi_expression[i]);
            }
            return *this;
        }
        template<typename RightExpr> auto operator &(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::LAND<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::LAND<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '|'/'|=' overload 
        template<typename RightExpr, typename std::enable_if<std::is_convertible_v<RightExpr, value_type>>::type* = nullptr>
        Container& operator |=(RightExpr&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] |= static_cast<value_type>(xi_expression);
            }
            return *this;
        }
        template<typename T, typename std::enable_if<Concepts::is_binary_expression_v<T>>::type* = nullptr>
        Container& operator |=(T&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] |= std::move(xi_expression[i]);
            }
            return *this;
        }
        template<typename RightExpr> auto operator &(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::LOR<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::LOR<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '^'/'^=' overload 
        template<typename RightExpr, typename std::enable_if<std::is_convertible_v<RightExpr, value_type>>::type* = nullptr>
        Container& operator ^=(RightExpr&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] ^= static_cast<value_type>(xi_expression);
            }
            return *this;
        }
        template<typename T, typename std::enable_if<Concepts::is_binary_expression_v<T>>::type* = nullptr>
        Container& operator ^=(T&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] ^= std::move(xi_expression[i]);
            }
            return *this;
        }
        template<typename RightExpr> auto operator ^(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::LXOR<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::LXOR<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '<<'/'<<=' overload 
        template<typename RightExpr, typename std::enable_if<std::is_convertible_v<RightExpr, value_type>>::type* = nullptr>
        Container& operator <<=(RightExpr&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] <<= static_cast<value_type>(xi_expression);
            }
            return *this;
        }
        template<typename T, typename std::enable_if<Concepts::is_binary_expression_v<T>>::type* = nullptr>
        Container& operator <<=(T&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] <<= std::move(xi_expression[i]);
            }
            return *this;
        }
        template<typename RightExpr> auto operator <<(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::SHL<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::SHL<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '>>'/'>>=' overload 
        template<typename RightExpr, typename std::enable_if<std::is_convertible_v<RightExpr, value_type>>::type* = nullptr>
        Container& operator >>=(RightExpr&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] >>= static_cast<value_type>(xi_expression);
            }
            return *this;
        }
        template<typename T, typename std::enable_if<Concepts::is_binary_expression_v<T>>::type* = nullptr>
        Container& operator >>=(T&& xi_expression) {
            for (std::size_t i{}; i < m_container.size(); ++i) {
                m_container[i] >>= std::move(xi_expression[i]);
            }
            return *this;
        }
        template<typename RightExpr> auto operator >>(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::SHR<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::SHR<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '=='
        template<typename RightExpr> auto operator ==(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::EQ<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::EQ<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '!='
        template<typename RightExpr> auto operator !=(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::NEQ<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::NEQ<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '<'
        template<typename RightExpr> auto operator <(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::LT<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::LT<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '<='
        template<typename RightExpr> auto operator <=(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::LE<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::LE<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '>'
        template<typename RightExpr> auto operator >(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::GT<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::GT<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // '>='
        template<typename RightExpr> auto operator >=(RightExpr&& xi_expression) const -> detail::BinaryExpression<const Container&, detail::BinaryOperations::GE<value_type>, decltype(std::forward<RightExpr>(xi_expression))> {
            return detail::BinaryExpression<const Container&, detail::BinaryOperations::GE<value_type>, decltype(std::forward<RightExpr>(xi_expression))>(*this, std::forward<RightExpr>(xi_expression));
        }

        // properties
        private:
            COLLECTION& m_container;
    };
};