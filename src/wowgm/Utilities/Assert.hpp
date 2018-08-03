#ifndef WOWGM_ASSERT_H_
#define WOWGM_ASSERT_H_

#include <iostream>

#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

#define BOOST_ENABLE_ASSERT_DEBUG_HANDLER
#include <boost/assert.hpp>

#include <boost/config/detail/suffix.hpp>

struct tag_stacktrace { };

typedef boost::error_info<tag_stacktrace, boost::stacktrace::stacktrace> traced;

namespace wowgm::exceptions
{
    template <class E, typename... Args>
    inline void throw_with_trace(Args&&... args)
    {
        E ex(std::forward<Args>(args)...);
        throw boost::enable_error_info(ex) << traced(boost::stacktrace::stacktrace());
    }
}

namespace boost
{
    namespace stacktrace
    {
        // More or less copypasted from boost.stacktrace. Frames within boost are skipped until we hit the first non-boost frame.
        class application_stacktrace
        {
            typedef basic_stacktrace<>::allocator_type Allocator;

            std::vector<boost::stacktrace::frame, Allocator> impl_;
            typedef boost::stacktrace::detail::native_frame_ptr_t native_frame_ptr_t;

            void fill(native_frame_ptr_t* begin, std::size_t size)
            {
                if (!size)
                    return;

                bool found_non_boost_yet = false;

                impl_.reserve(static_cast<std::size_t>(size));
                for (std::size_t i = 0; i < size; ++i)
                {
                    if (!begin[i])
                        return;

                    frame f(begin[i]);

                    auto name = f.name();

                    if (!found_non_boost_yet)
                    {
                        bool is_boost_frame = name.find("boost::") == 0;
                        found_non_boost_yet = !is_boost_frame; // Cant check npos since arguments can take boost::...

                        if (is_boost_frame)
                            continue;
                    }

                    impl_.push_back(f);
                }
            }

            static std::size_t frames_count_from_buffer_size(std::size_t buffer_size) BOOST_NOEXCEPT
            {
                const std::size_t ret = (buffer_size > sizeof(native_frame_ptr_t) ? buffer_size / sizeof(native_frame_ptr_t) : 0);
                return (ret > 1024 ? 1024 : ret); // Dealing with suspiciously big sizes
            }

            BOOST_NOINLINE void init(std::size_t frames_to_skip, std::size_t max_depth)
            {
                constexpr std::size_t buffer_size = 128;
                if (!max_depth)
                    return;

                try
                {
                    {   // Fast path without additional allocations
                        native_frame_ptr_t buffer[buffer_size];
                        const std::size_t frames_count = boost::stacktrace::detail::this_thread_frames::collect(buffer, buffer_size, frames_to_skip + 1);
                        if (buffer_size > frames_count || frames_count >= max_depth) {
                            const std::size_t size = (max_depth < frames_count ? max_depth : frames_count);
                            fill(buffer, size);
                            return;
                        }
                    }

                    // Failed to fit in `buffer_size`. Allocating memory:
                    typedef typename Allocator::template rebind<native_frame_ptr_t>::other allocator_void_t;
                    std::vector<native_frame_ptr_t, allocator_void_t> buf(buffer_size * 2, 0, impl_.get_allocator());
                    do {
                        const std::size_t frames_count = boost::stacktrace::detail::this_thread_frames::collect(&buf[0], buf.size(), frames_to_skip + 1);
                        if (buf.size() > frames_count || frames_count >= max_depth) {
                            const std::size_t size = (max_depth < frames_count ? max_depth : frames_count);
                            fill(&buf[0], size);
                            return;
                        }

                        buf.resize(buf.size() * 2);
                    } while (buf.size() < buf.max_size()); // close to `true`, but suppresses `C4127: conditional expression is constant`.
                }
                catch (...)
                {
                    // ignore exception
                }
            }
            /// @endcond

        public:
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::value_type             value_type;
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::allocator_type         allocator_type;
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::const_pointer          pointer;
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::const_pointer          const_pointer;
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::const_reference        reference;
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::const_reference        const_reference;
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::size_type              size_type;
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::difference_type        difference_type;
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::const_iterator         iterator;
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::const_iterator         const_iterator;
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::const_reverse_iterator reverse_iterator;
            typedef typename std::vector<boost::stacktrace::frame, Allocator>::const_reverse_iterator const_reverse_iterator;

            /// @brief Stores the current function call sequence inside *this without any decoding or any other heavy platform specific operations.
            ///
            /// @b Complexity: O(N) where N is call sequence length, O(1) if BOOST_STACKTRACE_USE_NOOP is defined.
            ///
            /// @b Async-Handler-Safety: Safe if Allocator construction, copying, Allocator::allocate and Allocator::deallocate are async signal safe.
            BOOST_FORCEINLINE application_stacktrace() BOOST_NOEXCEPT
                : impl_()
            {
                init(0, static_cast<std::size_t>(-1));
            }

            /// @brief Stores the current function call sequence inside *this without any decoding or any other heavy platform specific operations.
            ///
            /// @b Complexity: O(N) where N is call sequence length, O(1) if BOOST_STACKTRACE_USE_NOOP is defined.
            ///
            /// @b Async-Handler-Safety: Safe if Allocator construction, copying, Allocator::allocate and Allocator::deallocate are async signal safe.
            ///
            /// @param a Allocator that would be passed to underlying storeage.
            BOOST_FORCEINLINE explicit application_stacktrace(const allocator_type& a) BOOST_NOEXCEPT
                : impl_(a)
            {
                init(0, static_cast<std::size_t>(-1));
            }

            /// @brief Stores [skip, skip + max_depth) of the current function call sequence inside *this without any decoding or any other heavy platform specific operations.
            ///
            /// @b Complexity: O(N) where N is call sequence length, O(1) if BOOST_STACKTRACE_USE_NOOP is defined.
            ///
            /// @b Async-Handler-Safety: Safe if Allocator construction, copying, Allocator::allocate and Allocator::deallocate are async signal safe.
            ///
            /// @param skip How many top calls to skip and do not store in *this.
            ///
            /// @param max_depth Max call sequence depth to collect.
            ///
            /// @param a Allocator that would be passed to underlying storeage.
            ///
            /// @throws Nothing. Note that default construction of allocator may throw, however it is
            /// performed outside the constructor and exception in `allocator_type()` would not result in calling `std::terminate`.
            BOOST_FORCEINLINE application_stacktrace(std::size_t skip, std::size_t max_depth, const allocator_type& a = allocator_type()) BOOST_NOEXCEPT
                : impl_(a)
            {
                init(skip, max_depth);
            }

            /// @b Complexity: O(st.size())
            ///
            /// @b Async-Handler-Safety: Safe if Allocator construction, copying, Allocator::allocate and Allocator::deallocate are async signal safe.
            application_stacktrace(const application_stacktrace& st)
                : impl_(st.impl_)
            {}

            /// @b Complexity: O(st.size())
            ///
            /// @b Async-Handler-Safety: Safe if Allocator construction, copying, Allocator::allocate and Allocator::deallocate are async signal safe.
            application_stacktrace& operator=(const application_stacktrace& st) {
                impl_ = st.impl_;
                return *this;
            }

#ifdef BOOST_STACKTRACE_DOXYGEN_INVOKED
            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe if Allocator::deallocate is async signal safe.
            ~application_stacktrace() BOOST_NOEXCEPT = default;
#endif

#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe if Allocator construction and copying are async signal safe.
            application_stacktrace(application_stacktrace&& st) BOOST_NOEXCEPT
                : impl_(std::move(st.impl_))
            {}

            /// @b Complexity: O(st.size())
            ///
            /// @b Async-Handler-Safety: Safe if Allocator construction and copying are async signal safe.
            application_stacktrace& operator=(application_stacktrace&& st)
#ifndef BOOST_NO_CXX11_HDR_TYPE_TRAITS
                BOOST_NOEXCEPT_IF((std::is_nothrow_move_assignable< std::vector<boost::stacktrace::frame, Allocator> >::value))
#else
                BOOST_NOEXCEPT
#endif
            {
                impl_ = std::move(st.impl_);
                return *this;
            }
#endif

            /// @returns Number of function names stored inside the class.
            ///
            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe.
            size_type size() const BOOST_NOEXCEPT {
                return impl_.size();
            }

            /// @param frame_no Zero based index of frame to return. 0
            /// is the function index where stacktrace was constructed and
            /// index close to this->size() contains function `main()`.
            /// @returns frame that references the actual frame info, stored inside *this.
            ///
            /// @b Complexity: O(1).
            ///
            /// @b Async-Handler-Safety: Safe.
            const_reference operator[](std::size_t frame_no) const BOOST_NOEXCEPT {
                return impl_[frame_no];
            }

            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe.
            const_iterator begin() const BOOST_NOEXCEPT { return impl_.begin(); }
            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe.
            const_iterator cbegin() const BOOST_NOEXCEPT { return impl_.begin(); }
            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe.
            const_iterator end() const BOOST_NOEXCEPT { return impl_.end(); }
            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe.
            const_iterator cend() const BOOST_NOEXCEPT { return impl_.end(); }

            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe.
            const_reverse_iterator rbegin() const BOOST_NOEXCEPT { return impl_.rbegin(); }
            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe.
            const_reverse_iterator crbegin() const BOOST_NOEXCEPT { return impl_.rbegin(); }
            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe.
            const_reverse_iterator rend() const BOOST_NOEXCEPT { return impl_.rend(); }
            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe.
            const_reverse_iterator crend() const BOOST_NOEXCEPT { return impl_.rend(); }


            /// @brief Allows to check that stack trace capturing was successful.
            /// @returns `true` if `this->size() != 0`
            ///
            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe.
            BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()

            /// @brief Allows to check that stack trace failed.
            /// @returns `true` if `this->size() == 0`
            ///
            /// @b Complexity: O(1)
            ///
            /// @b Async-Handler-Safety: Safe.
            bool empty() const BOOST_NOEXCEPT { return !size(); }

            /// @cond
            bool operator!() const BOOST_NOEXCEPT { return !size(); }
            /// @endcond

            const std::vector<boost::stacktrace::frame, Allocator>& as_vector() const BOOST_NOEXCEPT
            {
                return impl_;
            }

            /// Constructs stacktrace from basic_istreamable that references the dumped stacktrace. Terminating zero frame is discarded.
            ///
            /// @b Complexity: O(N)
            template <class Char, class Trait>
            static application_stacktrace from_dump(std::basic_istream<Char, Trait>& in, const allocator_type& a = allocator_type())
            {
                typedef typename std::basic_istream<Char, Trait>::pos_type pos_type;
                application_stacktrace ret(0, 0, a);

                // reserving space
                const pos_type pos = in.tellg();
                in.seekg(0, in.end);
                const std::size_t frames_count = frames_count_from_buffer_size(static_cast<std::size_t>(in.tellg()));
                in.seekg(pos);

                if (!frames_count)
                    return ret;

                native_frame_ptr_t ptr = 0;
                ret.impl_.reserve(frames_count);
                while (in.read(reinterpret_cast<Char*>(&ptr), sizeof(ptr)))
                {
                    if (!ptr)
                        break;

                    ret.impl_.push_back(frame(ptr));
                }

                return ret;
            }

            /// Constructs stacktrace from raw memory dump. Terminating zero frame is discarded.
            ///
            /// @b Complexity: O(size) in worst case
            static application_stacktrace from_dump(const void* begin, std::size_t buffer_size_in_bytes, const allocator_type& a = allocator_type())
            {
                application_stacktrace ret(0, 0, a);
                const native_frame_ptr_t* first = static_cast<const native_frame_ptr_t*>(begin);
                const std::size_t frames_count = frames_count_from_buffer_size(buffer_size_in_bytes);
                if (!frames_count)
                    return ret;

                const native_frame_ptr_t* const last = first + frames_count;
                ret.impl_.reserve(frames_count);
                for (; first != last; ++first)
                {
                    if (!*first)
                        break;

                    ret.impl_.push_back(frame(*first));
                }

                return ret;
            }
        };

        /// Outputs stacktrace in a human readable format to output stream; unsafe to use in async handlers.
        template <class CharT, class TraitsT>
        std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, const application_stacktrace& bt)
        {
            if (bt)
                os << boost::stacktrace::detail::to_string(&bt.as_vector()[0], bt.size());

            return os;
        }
    }

    inline void assertion_failed_msg_fmt(char const* expr, char const* msg, char const* function, char const* file, long line, ...)
    {
        va_list args;
        va_start(args, line);

        std::cerr << file << ':' << line << " in " << function << " ASSERTION FAILED \"" << expr << "\" \n";
        vfprintf(stderr, msg, args);
        std::cerr << '\n';
        std::cerr << "Backtrace:\n" << boost::stacktrace::application_stacktrace() << std::endl;

        va_end(args);
            
        *((volatile int*)NULL) = 0;
        exit(1);
    }

    inline void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, long line) {

        std::cerr << file << ':' << line << " in " << function << " ASSERTION FAILED \"" << expr << "\" \n";
        if (msg != nullptr)
            std::cerr << msg << '\n';

        std::cerr << "Backtrace:\n" << boost::stacktrace::application_stacktrace() << '\n';

        std::exit(1);
    }

    inline void assertion_failed(char const* expr, char const* function, char const* file, long line) {
        ::boost::assertion_failed_msg(expr, 0 /*nullptr*/, function, file, line);
    }
} // namespace boost

#endif // WOWGM_ASSERT_H_

#define BOOST_ASSERT_MSG_FMT(expr, msg, ...) (BOOST_LIKELY(!!(expr))? ((void)0): ::boost::assertion_failed_msg_fmt(#expr, msg, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__, ##__VA_ARGS__))

